// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/BasePlayerState.h"

#include "AbilitySystem/RiftAbilitySystemComponent.h"
#include "AbilitySystem/Attributes/RiftPlayerAttributeSet.h"
#include "AbilitySystem/Attributes/RiftResourceAttributeSet.h"
#include "Data/Player/PlayerClassConfig.h"
#include "Engine/World.h"
#include "GameFramework/GameStateBase.h"
#include "Net/UnrealNetwork.h"

ABasePlayerState::ABasePlayerState()
{
	AbilitySystemComponent = CreateDefaultSubobject<URiftAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	AttributeSet = CreateDefaultSubobject<URiftPlayerAttributeSet>(TEXT("AttributeSet"));
	ResourceAttributeSet = CreateDefaultSubobject<URiftResourceAttributeSet>(TEXT("ResourceAttributeSet"));

	SetNetUpdateFrequency(100.0f);
}

UAbilitySystemComponent* ABasePlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void ABasePlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABasePlayerState, SelectedPlayerClassConfig);
	DOREPLIFETIME(ABasePlayerState, bIsRoomHost);
	DOREPLIFETIME(ABasePlayerState, bIsWaitingForRespawn);
	DOREPLIFETIME(ABasePlayerState, RespawnEndServerTime);
	DOREPLIFETIME(ABasePlayerState, RespawnDuration);
}

void ABasePlayerState::CopyProperties(APlayerState* PlayerState)
{
	Super::CopyProperties(PlayerState);

	ABasePlayerState* NewPlayerState = Cast<ABasePlayerState>(PlayerState);
	if (!NewPlayerState)
	{
		return;
	}

	NewPlayerState->SelectedPlayerClassConfig = SelectedPlayerClassConfig;
	NewPlayerState->bIsRoomHost = bIsRoomHost;
}

void ABasePlayerState::SetSelectedPlayerClassConfig(UPlayerClassConfig* NewPlayerClassConfig)
{
	if (!HasAuthority() || SelectedPlayerClassConfig == NewPlayerClassConfig)
	{
		return;
	}

	SelectedPlayerClassConfig = NewPlayerClassConfig;
	OnLobbyPlayerStateChanged.Broadcast();
	ForceNetUpdate();
}

void ABasePlayerState::SetIsRoomHost(const bool bNewIsRoomHost)
{
	if (!HasAuthority() || bIsRoomHost == bNewIsRoomHost)
	{
		return;
	}

	bIsRoomHost = bNewIsRoomHost;
	OnLobbyPlayerStateChanged.Broadcast();
	ForceNetUpdate();
}

float ABasePlayerState::GetRespawnRemainingTime() const
{
	if (!bIsWaitingForRespawn)
	{
		return 0.0f;
	}

	float CurrentServerTime = 0.0f;
	const UWorld* World = GetWorld();
	const AGameStateBase* CurrentGameState = World ? World->GetGameState<AGameStateBase>() : nullptr;
	if (CurrentGameState)
	{
		CurrentServerTime = CurrentGameState->GetServerWorldTimeSeconds();
	}
	else if (World)
	{
		CurrentServerTime = World->GetTimeSeconds();
	}

	return FMath::Max(0.0f, RespawnEndServerTime - CurrentServerTime);
}

void ABasePlayerState::SetRespawnState(const bool bWaiting, const float EndServerTime, const float Duration)
{
	if (!HasAuthority())
	{
		return;
	}

	const float ClampedDuration = FMath::Max(0.0f, Duration);
	if (bIsWaitingForRespawn == bWaiting &&
		FMath::IsNearlyEqual(RespawnEndServerTime, EndServerTime) &&
		FMath::IsNearlyEqual(RespawnDuration, ClampedDuration))
	{
		return;
	}

	bIsWaitingForRespawn = bWaiting;
	RespawnEndServerTime = EndServerTime;
	RespawnDuration = ClampedDuration;
	OnRespawnStateChanged.Broadcast();
	ForceNetUpdate();
}

void ABasePlayerState::OnRep_SelectedPlayerClassConfig()
{
	OnLobbyPlayerStateChanged.Broadcast();
}

void ABasePlayerState::OnRep_IsRoomHost()
{
	OnLobbyPlayerStateChanged.Broadcast();
}

void ABasePlayerState::OnRep_RespawnState()
{
	OnRespawnStateChanged.Broadcast();
}
