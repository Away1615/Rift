// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Lobby/RiftLobbyWidget.h"

#include "Core/RiftLobbyGameState.h"
#include "Engine/World.h"
#include "Player/BasePlayerController.h"
#include "Player/BasePlayerState.h"

void URiftLobbyWidget::NativeConstruct()
{
	Super::NativeConstruct();

	BindToPlayerController();
	BindToLobbyGameState();
	RefreshLobbyState();
}

void URiftLobbyWidget::NativeDestruct()
{
	UnbindFromLobbyPlayerStates();
	UnbindFromLobbyGameState();
	UnbindFromPlayerController();

	Super::NativeDestruct();
}

FString URiftLobbyWidget::GetRoomCode() const
{
	const UWorld* World = GetWorld();
	const ARiftLobbyGameState* LobbyGameState = World ? World->GetGameState<ARiftLobbyGameState>() : nullptr;
	return LobbyGameState ? LobbyGameState->GetRoomCode() : FString();
}

bool URiftLobbyWidget::IsLocalPlayerRoomHost() const
{
	const ABasePlayerController* RiftPlayerController = GetRiftPlayerController();
	const ABasePlayerState* RiftPlayerState = RiftPlayerController
		? RiftPlayerController->GetPlayerState<ABasePlayerState>()
		: nullptr;

	return RiftPlayerState && RiftPlayerState->IsRoomHost();
}

UPlayerClassConfig* URiftLobbyWidget::GetSelectedPlayerClass() const
{
	const ABasePlayerController* RiftPlayerController = GetRiftPlayerController();
	const ABasePlayerState* RiftPlayerState = RiftPlayerController
		? RiftPlayerController->GetPlayerState<ABasePlayerState>()
		: nullptr;

	return RiftPlayerState ? RiftPlayerState->GetSelectedPlayerClassConfig() : nullptr;
}

void URiftLobbyWidget::SelectPlayerClass(UPlayerClassConfig* ClassConfig)
{
	if (!ClassConfig)
	{
		return;
	}

	ABasePlayerController* RiftPlayerController = GetRiftPlayerController();
	if (!RiftPlayerController)
	{
		return;
	}

	RiftPlayerController->RequestSelectPlayerClass(ClassConfig);
	RefreshLobbyState();
}

void URiftLobbyWidget::StartGame()
{
	ABasePlayerController* RiftPlayerController = GetRiftPlayerController();
	if (!RiftPlayerController)
	{
		return;
	}

	RiftPlayerController->RequestStartLobbyGame();
	RefreshLobbyState();
}

void URiftLobbyWidget::RefreshLobbyState()
{
	BindToLobbyGameState();
	UnbindFromLobbyPlayerStates();
	BindToLobbyPlayerStates();

	OnLobbyStateRefreshed();
}

ABasePlayerController* URiftLobbyWidget::GetRiftPlayerController() const
{
	return Cast<ABasePlayerController>(GetOwningPlayer());
}

void URiftLobbyWidget::BindToPlayerController()
{
	ABasePlayerController* RiftPlayerController = GetRiftPlayerController();
	if (!RiftPlayerController)
	{
		return;
	}

	BoundPlayerController = RiftPlayerController;
	RiftPlayerController->OnLobbyStartTransitionRequested.AddUniqueDynamic(
		this,
		&URiftLobbyWidget::HandleLobbyStartTransition
	);
	RiftPlayerController->OnLobbyActionFailedRequested.AddUniqueDynamic(
		this,
		&URiftLobbyWidget::HandleLobbyActionFailed
	);
}

void URiftLobbyWidget::UnbindFromPlayerController()
{
	ABasePlayerController* RiftPlayerController = BoundPlayerController.Get();
	if (!RiftPlayerController)
	{
		return;
	}

	RiftPlayerController->OnLobbyStartTransitionRequested.RemoveDynamic(
		this,
		&URiftLobbyWidget::HandleLobbyStartTransition
	);
	RiftPlayerController->OnLobbyActionFailedRequested.RemoveDynamic(
		this,
		&URiftLobbyWidget::HandleLobbyActionFailed
	);
	BoundPlayerController.Reset();
}

void URiftLobbyWidget::BindToLobbyGameState()
{
	UWorld* World = GetWorld();
	ARiftLobbyGameState* LobbyGameState = World ? World->GetGameState<ARiftLobbyGameState>() : nullptr;
	if (!LobbyGameState)
	{
		return;
	}

	if (BoundLobbyGameState.Get() == LobbyGameState)
	{
		return;
	}

	UnbindFromLobbyGameState();
	BoundLobbyGameState = LobbyGameState;
	LobbyGameState->OnLobbyGameStateChanged.AddUniqueDynamic(
		this,
		&URiftLobbyWidget::HandleLobbyGameStateChanged
	);
}

void URiftLobbyWidget::UnbindFromLobbyGameState()
{
	ARiftLobbyGameState* LobbyGameState = BoundLobbyGameState.Get();
	if (!LobbyGameState)
	{
		BoundLobbyGameState.Reset();
		return;
	}

	LobbyGameState->OnLobbyGameStateChanged.RemoveDynamic(
		this,
		&URiftLobbyWidget::HandleLobbyGameStateChanged
	);
	BoundLobbyGameState.Reset();
}

void URiftLobbyWidget::BindToLobbyPlayerStates()
{
	UWorld* World = GetWorld();
	const ARiftLobbyGameState* LobbyGameState = World ? World->GetGameState<ARiftLobbyGameState>() : nullptr;
	if (!LobbyGameState)
	{
		return;
	}

	for (APlayerState* PlayerState : LobbyGameState->PlayerArray)
	{
		ABasePlayerState* RiftPlayerState = Cast<ABasePlayerState>(PlayerState);
		if (!RiftPlayerState)
		{
			continue;
		}

		RiftPlayerState->OnLobbyPlayerStateChanged.AddUniqueDynamic(
			this,
			&URiftLobbyWidget::HandleLobbyPlayerStateChanged
		);
		BoundPlayerStates.AddUnique(RiftPlayerState);
	}
}

void URiftLobbyWidget::UnbindFromLobbyPlayerStates()
{
	for (const TWeakObjectPtr<ABasePlayerState>& PlayerStatePtr : BoundPlayerStates)
	{
		ABasePlayerState* RiftPlayerState = PlayerStatePtr.Get();
		if (!RiftPlayerState)
		{
			continue;
		}

		RiftPlayerState->OnLobbyPlayerStateChanged.RemoveDynamic(
			this,
			&URiftLobbyWidget::HandleLobbyPlayerStateChanged
		);
	}

	BoundPlayerStates.Empty();
}

void URiftLobbyWidget::HandleLobbyGameStateChanged()
{
	RefreshLobbyState();
}

void URiftLobbyWidget::HandleLobbyPlayerStateChanged()
{
	RefreshLobbyState();
}

void URiftLobbyWidget::HandleLobbyStartTransition(const float Duration)
{
	OnLobbyStartTransition(Duration);
}

void URiftLobbyWidget::HandleLobbyActionFailed(const FString& ErrorMessage)
{
	OnLobbyError(ErrorMessage);
}
