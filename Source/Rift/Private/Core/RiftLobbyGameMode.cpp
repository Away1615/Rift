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
	SyncDefaultPlayerClassConfigToGameState();
	RefreshAllPlayersReady();
}

void ARiftLobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	ABasePlayerState* RiftPlayerState = NewPlayer ? NewPlayer->GetPlayerState<ABasePlayerState>() : nullptr;
	ABasePlayerController* RiftPlayerController = Cast<ABasePlayerController>(NewPlayer);
	if (RiftPlayerState)
	{
		AssignLobbySlot(RiftPlayerState);
		if (RiftPlayerState->GetLobbySlotIndex() == INDEX_NONE)
		{
			FailLobbyAction(RiftPlayerController, TEXT("Lobby is full."));
		}

		if (!RiftPlayerState->GetSelectedPlayerClassConfig() && DefaultPlayerClassConfig)
		{
			RiftPlayerState->SetSelectedPlayerClassConfig(DefaultPlayerClassConfig);
		}
	}

	if (RiftPlayerState && !HasRoomHost())
	{
		RiftPlayerState->SetIsRoomHost(true);
	}

	SyncRoomCodeToGameState();
	SyncDefaultPlayerClassConfigToGameState();
	RefreshAllPlayersReady();

	const ARiftLobbyGameState* LobbyGameState = GetGameState<ARiftLobbyGameState>();
	if (LobbyGameState && LobbyGameState->IsStarting() && RiftPlayerController)
	{
		RiftPlayerController->Client_PlayLobbyStartTransition(LobbyGameState->GetStartTransitionDuration());
	}
}

void ARiftLobbyGameMode::Logout(AController* Exiting)
{
	ABasePlayerState* ExitingPlayerState = Exiting ? Exiting->GetPlayerState<ABasePlayerState>() : nullptr;
	const bool bWasRoomHost = ExitingPlayerState && ExitingPlayerState->IsRoomHost();
	ReleaseLobbySlot(ExitingPlayerState);

	Super::Logout(Exiting);

	const ARiftLobbyGameState* LobbyGameState = GetGameState<ARiftLobbyGameState>();
	if (bWasRoomHost && LobbyGameState && !LobbyGameState->IsStarting())
	{
		AssignFirstAvailableHost();
	}

	RefreshAllPlayersReady();
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

	if (!DefaultPlayerClassConfig)
	{
		FailLobbyAction(RequestingController, TEXT("Player class is not configured."));
		return;
	}

	if (!AreAllPlayersReadyForGameplay())
	{
		FailLobbyAction(RequestingController, TEXT("Not all players have finished character creation."));
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

int32 ARiftLobbyGameMode::FindAvailableLobbySlotIndex() const
{
	const AGameStateBase* CurrentGameState = GameState;
	if (!CurrentGameState)
	{
		return INDEX_NONE;
	}

	for (int32 CandidateSlotIndex = 0; CandidateSlotIndex < 4; ++CandidateSlotIndex)
	{
		bool bIsOccupied = false;
		for (APlayerState* PlayerState : CurrentGameState->PlayerArray)
		{
			const ABasePlayerState* RiftPlayerState = Cast<ABasePlayerState>(PlayerState);
			if (RiftPlayerState && RiftPlayerState->GetLobbySlotIndex() == CandidateSlotIndex)
			{
				bIsOccupied = true;
				break;
			}
		}

		if (!bIsOccupied)
		{
			return CandidateSlotIndex;
		}
	}

	return INDEX_NONE;
}

void ARiftLobbyGameMode::AssignLobbySlot(ABasePlayerState* PlayerState)
{
	if (!PlayerState)
	{
		return;
	}

	PlayerState->SetLobbySlotIndex(FindAvailableLobbySlotIndex());
}

void ARiftLobbyGameMode::ReleaseLobbySlot(ABasePlayerState* PlayerState)
{
	if (!PlayerState)
	{
		return;
	}

	PlayerState->SetLobbySlotIndex(INDEX_NONE);
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

void ARiftLobbyGameMode::RefreshAllPlayersReady()
{
	ARiftLobbyGameState* LobbyGameState = GetGameState<ARiftLobbyGameState>();
	if (!LobbyGameState)
	{
		return;
	}

	LobbyGameState->SetAllPlayersReady(AreAllPlayersConfirmed());
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

void ARiftLobbyGameMode::SyncDefaultPlayerClassConfigToGameState() const
{
	ARiftLobbyGameState* LobbyGameState = GetGameState<ARiftLobbyGameState>();
	if (!LobbyGameState)
	{
		return;
	}

	LobbyGameState->SetDefaultPlayerClassConfig(DefaultPlayerClassConfig);
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

bool ARiftLobbyGameMode::AreAllPlayersConfirmed() const
{
	const AGameStateBase* CurrentGameState = GameState;
	if (!CurrentGameState || CurrentGameState->PlayerArray.Num() <= 0)
	{
		return false;
	}

	for (APlayerState* PlayerState : CurrentGameState->PlayerArray)
	{
		const ABasePlayerState* RiftPlayerState = Cast<ABasePlayerState>(PlayerState);
		if (!RiftPlayerState || !RiftPlayerState->IsLobbyCharacterConfirmed())
		{
			return false;
		}
	}

	return true;
}

bool ARiftLobbyGameMode::AreAllPlayersReadyForGameplay() const
{
	const AGameStateBase* CurrentGameState = GameState;
	if (!CurrentGameState || !DefaultPlayerClassConfig)
	{
		return false;
	}

	for (APlayerState* PlayerState : CurrentGameState->PlayerArray)
	{
		const ABasePlayerState* RiftPlayerState = Cast<ABasePlayerState>(PlayerState);
		if (!RiftPlayerState ||
			!RiftPlayerState->GetSelectedPlayerClassConfig() ||
			!RiftPlayerState->IsLobbyCharacterConfirmed())
		{
			return false;
		}
	}

	return CurrentGameState->PlayerArray.Num() > 0;
}
