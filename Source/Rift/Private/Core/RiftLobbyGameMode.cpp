// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/RiftLobbyGameMode.h"

#include "Core/RiftGameInstance.h"
#include "Core/RiftLobbyGameState.h"
#include "Engine/World.h"
#include "Player/BasePlayerController.h"
#include "Player/BasePlayerState.h"

ARiftLobbyGameMode::ARiftLobbyGameMode()
{
	PlayerControllerClass = ABasePlayerController::StaticClass();
	PlayerStateClass = ABasePlayerState::StaticClass();
	GameStateClass = ARiftLobbyGameState::StaticClass();
	DefaultPawnClass = nullptr;
	bStartPlayersAsSpectators = true;
	bUseSeamlessTravel = true;
}

void ARiftLobbyGameMode::InitGameState()
{
	Super::InitGameState();

	SyncRoomCodeToGameState();
}

void ARiftLobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	ABasePlayerState* RiftPlayerState = NewPlayer ? NewPlayer->GetPlayerState<ABasePlayerState>() : nullptr;
	if (RiftPlayerState && !HasRoomHost())
	{
		RiftPlayerState->SetIsRoomHost(true);
	}

	SyncRoomCodeToGameState();

	const ARiftLobbyGameState* LobbyGameState = GetGameState<ARiftLobbyGameState>();
	ABasePlayerController* RiftPlayerController = Cast<ABasePlayerController>(NewPlayer);
	if (LobbyGameState && LobbyGameState->IsStarting() && RiftPlayerController)
	{
		RiftPlayerController->Client_PlayLobbyStartTransition(LobbyGameState->GetStartTransitionDuration());
	}
}

void ARiftLobbyGameMode::Logout(AController* Exiting)
{
	const ABasePlayerState* ExitingPlayerState = Exiting ? Exiting->GetPlayerState<ABasePlayerState>() : nullptr;
	const bool bWasRoomHost = ExitingPlayerState && ExitingPlayerState->IsRoomHost();

	Super::Logout(Exiting);

	const ARiftLobbyGameState* LobbyGameState = GetGameState<ARiftLobbyGameState>();
	if (bWasRoomHost && LobbyGameState && !LobbyGameState->IsStarting())
	{
		AssignFirstAvailableHost();
	}
}

void ARiftLobbyGameMode::StartGameFromLobby(ABasePlayerController* RequestingController)
{
	const ABasePlayerState* RequestingPlayerState = RequestingController
		? RequestingController->GetPlayerState<ABasePlayerState>()
		: nullptr;

	if (!RequestingPlayerState || !RequestingPlayerState->IsRoomHost())
	{
		FailLobbyAction(RequestingController, TEXT("Only the room host can start the game."));
		return;
	}

	const ARiftLobbyGameState* LobbyGameState = GetGameState<ARiftLobbyGameState>();
	if (!LobbyGameState)
	{
		FailLobbyAction(RequestingController, TEXT("Lobby state is unavailable."));
		return;
	}

	if (LobbyGameState->IsStarting())
	{
		FailLobbyAction(RequestingController, TEXT("Game is already starting."));
		return;
	}

	if (!AreAllPlayersReadyForGameplay())
	{
		FailLobbyAction(RequestingController, TEXT("All players must select a class before starting."));
		return;
	}

	const URiftGameInstance* RiftGameInstance = GetGameInstance<URiftGameInstance>();
	if (!RiftGameInstance || !RiftGameInstance->HasValidGameplayMap())
	{
		FailLobbyAction(RequestingController, TEXT("Gameplay map is not configured."));
		return;
	}

	BeginStartTransition();
}

void ARiftLobbyGameMode::BeginStartTransition()
{
	ARiftLobbyGameState* LobbyGameState = GetGameState<ARiftLobbyGameState>();
	if (!LobbyGameState || LobbyGameState->IsStarting())
	{
		return;
	}

	const float TransitionDuration = FMath::Max(0.0f, StartTransitionDuration);
	LobbyGameState->SetStartTransitionDuration(TransitionDuration);
	LobbyGameState->SetLobbyState(ERiftLobbyState::Starting);

	NotifyPlayersStartTransition();

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	if (TransitionDuration <= 0.0f)
	{
		TravelToGameplayMap();
		return;
	}

	World->GetTimerManager().SetTimer(
		StartGameTravelTimerHandle,
		this,
		&ARiftLobbyGameMode::TravelToGameplayMap,
		TransitionDuration,
		false
	);
}

void ARiftLobbyGameMode::NotifyPlayersStartTransition() const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const ARiftLobbyGameState* LobbyGameState = GetGameState<ARiftLobbyGameState>();
	const float TransitionDuration = LobbyGameState ? LobbyGameState->GetStartTransitionDuration() : StartTransitionDuration;

	for (FConstPlayerControllerIterator Iterator = World->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		ABasePlayerController* RiftPlayerController = Cast<ABasePlayerController>(Iterator->Get());
		if (RiftPlayerController)
		{
			RiftPlayerController->Client_PlayLobbyStartTransition(TransitionDuration);
		}
	}
}

void ARiftLobbyGameMode::TravelToGameplayMap()
{
	const URiftGameInstance* RiftGameInstance = GetGameInstance<URiftGameInstance>();
	if (!RiftGameInstance || !RiftGameInstance->HasValidGameplayMap())
	{
		UE_LOG(LogTemp, Warning, TEXT("TravelToGameplayMap rejected: gameplay map is not configured."));
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	World->ServerTravel(RiftGameInstance->GetGameplayMapPath() + TEXT("?listen"));
}

void ARiftLobbyGameMode::FailLobbyAction(
	ABasePlayerController* RequestingController,
	const FString& ErrorMessage
) const
{
	UE_LOG(LogTemp, Warning, TEXT("Lobby action failed: %s"), *ErrorMessage);

	if (RequestingController)
	{
		RequestingController->Client_LobbyActionFailed(ErrorMessage);
	}
}

void ARiftLobbyGameMode::AssignFirstAvailableHost()
{
	AGameStateBase* CurrentGameState = GameState;
	if (!CurrentGameState)
	{
		return;
	}

	ABasePlayerState* NewHostPlayerState = nullptr;
	for (APlayerState* PlayerState : CurrentGameState->PlayerArray)
	{
		ABasePlayerState* RiftPlayerState = Cast<ABasePlayerState>(PlayerState);
		if (!RiftPlayerState)
		{
			continue;
		}

		if (!NewHostPlayerState)
		{
			NewHostPlayerState = RiftPlayerState;
		}

		RiftPlayerState->SetIsRoomHost(false);
	}

	if (NewHostPlayerState)
	{
		NewHostPlayerState->SetIsRoomHost(true);
	}
}

void ARiftLobbyGameMode::SyncRoomCodeToGameState() const
{
	ARiftLobbyGameState* LobbyGameState = GetGameState<ARiftLobbyGameState>();
	const URiftGameInstance* RiftGameInstance = GetGameInstance<URiftGameInstance>();
	if (!LobbyGameState || !RiftGameInstance)
	{
		return;
	}

	LobbyGameState->SetRoomCode(RiftGameInstance->GetCurrentRoomCode());
}

bool ARiftLobbyGameMode::HasRoomHost() const
{
	const AGameStateBase* CurrentGameState = GameState;
	if (!CurrentGameState)
	{
		return false;
	}

	for (APlayerState* PlayerState : CurrentGameState->PlayerArray)
	{
		const ABasePlayerState* RiftPlayerState = Cast<ABasePlayerState>(PlayerState);
		if (RiftPlayerState && RiftPlayerState->IsRoomHost())
		{
			return true;
		}
	}

	return false;
}

bool ARiftLobbyGameMode::AreAllPlayersReadyForGameplay() const
{
	const AGameStateBase* CurrentGameState = GameState;
	if (!CurrentGameState)
	{
		return false;
	}

	for (APlayerState* PlayerState : CurrentGameState->PlayerArray)
	{
		const ABasePlayerState* RiftPlayerState = Cast<ABasePlayerState>(PlayerState);
		if (!RiftPlayerState || !RiftPlayerState->GetSelectedPlayerClassConfig())
		{
			return false;
		}
	}

	return CurrentGameState->PlayerArray.Num() > 0;
}
