// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/RiftLobbyGameState.h"

#include "Net/UnrealNetwork.h"

void ARiftLobbyGameState::SetRoomCode(const FString& NewRoomCode)
{
	if (!HasAuthority() || RoomCode == NewRoomCode)
	{
		return;
	}

	RoomCode = NewRoomCode;
	OnLobbyGameStateChanged.Broadcast();
	ForceNetUpdate();
}

void ARiftLobbyGameState::SetLobbyState(const ERiftLobbyState NewState)
{
	if (!HasAuthority() || LobbyState == NewState)
	{
		return;
	}

	LobbyState = NewState;
	OnLobbyGameStateChanged.Broadcast();
	ForceNetUpdate();
}

void ARiftLobbyGameState::SetStartTransitionDuration(const float NewDuration)
{
	if (!HasAuthority())
	{
		return;
	}

	const float ClampedDuration = FMath::Max(0.0f, NewDuration);
	if (FMath::IsNearlyEqual(StartTransitionDuration, ClampedDuration))
	{
		return;
	}

	StartTransitionDuration = ClampedDuration;
	OnLobbyGameStateChanged.Broadcast();
	ForceNetUpdate();
}

void ARiftLobbyGameState::SetDefaultPlayerClassConfig(UPlayerClassConfig* NewDefaultPlayerClassConfig)
{
	if (!HasAuthority() || DefaultPlayerClassConfig == NewDefaultPlayerClassConfig)
	{
		return;
	}

	DefaultPlayerClassConfig = NewDefaultPlayerClassConfig;
	OnLobbyGameStateChanged.Broadcast();
	ForceNetUpdate();
}

void ARiftLobbyGameState::SetAllPlayersReady(const bool bNewAllPlayersReady)
{
	if (!HasAuthority() || bAllPlayersReady == bNewAllPlayersReady)
	{
		return;
	}

	bAllPlayersReady = bNewAllPlayersReady;
	OnLobbyGameStateChanged.Broadcast();
	OnLobbyAllPlayersReadyChanged.Broadcast(bAllPlayersReady);
	ForceNetUpdate();
}

void ARiftLobbyGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ARiftLobbyGameState, RoomCode);
	DOREPLIFETIME(ARiftLobbyGameState, LobbyState);
	DOREPLIFETIME(ARiftLobbyGameState, StartTransitionDuration);
	DOREPLIFETIME(ARiftLobbyGameState, DefaultPlayerClassConfig);
	DOREPLIFETIME(ARiftLobbyGameState, bAllPlayersReady);
}

void ARiftLobbyGameState::AddPlayerState(APlayerState* PlayerState)
{
	Super::AddPlayerState(PlayerState);

	OnLobbyGameStateChanged.Broadcast();
}

void ARiftLobbyGameState::RemovePlayerState(APlayerState* PlayerState)
{
	Super::RemovePlayerState(PlayerState);

	OnLobbyGameStateChanged.Broadcast();
}

void ARiftLobbyGameState::OnRep_RoomCode()
{
	OnLobbyGameStateChanged.Broadcast();
}

void ARiftLobbyGameState::OnRep_LobbyState()
{
	OnLobbyGameStateChanged.Broadcast();
}

void ARiftLobbyGameState::OnRep_StartTransitionDuration()
{
	OnLobbyGameStateChanged.Broadcast();
}

void ARiftLobbyGameState::OnRep_DefaultPlayerClassConfig()
{
	OnLobbyGameStateChanged.Broadcast();
}

void ARiftLobbyGameState::OnRep_AllPlayersReady()
{
	OnLobbyGameStateChanged.Broadcast();
	OnLobbyAllPlayersReadyChanged.Broadcast(bAllPlayersReady);
}
