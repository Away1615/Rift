// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Player/PlayerHUDWidget.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystem/Attributes/RiftPlayerAttributeSet.h"
#include "AbilitySystem/Attributes/RiftResourceAttributeSet.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Player/BasePlayerState.h"
#include "TimerManager.h"

void UPlayerHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	TryInitialize();
}

void UPlayerHUDWidget::NativeDestruct()
{
	if (UAbilitySystemComponent* ASC = AbilitySystemComponent.Get())
	{
		ASC->GetGameplayAttributeValueChangeDelegate(URiftPlayerAttributeSet::GetHealthAttribute()).RemoveAll(this);
		ASC->GetGameplayAttributeValueChangeDelegate(URiftPlayerAttributeSet::GetMaxHealthAttribute()).RemoveAll(this);
		ASC->GetGameplayAttributeValueChangeDelegate(URiftPlayerAttributeSet::GetStaminaAttribute()).RemoveAll(this);
		ASC->GetGameplayAttributeValueChangeDelegate(URiftPlayerAttributeSet::GetMaxStaminaAttribute()).RemoveAll(this);
		ASC->GetGameplayAttributeValueChangeDelegate(URiftResourceAttributeSet::GetUltimateChargeAttribute()).RemoveAll(this);
		ASC->GetGameplayAttributeValueChangeDelegate(URiftResourceAttributeSet::GetMaxUltimateChargeAttribute()).RemoveAll(this);
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(InitRetryTimerHandle);
		World->GetTimerManager().ClearTimer(RespawnCountdownTimerHandle);
	}

	UnbindFromPlayerState();

	Super::NativeDestruct();
}

void UPlayerHUDWidget::TryInitialize()
{
	if (bInitialized) return;

	UAbilitySystemComponent* ASC = nullptr;
	ABasePlayerState* RiftPlayerState = nullptr;
	if (APlayerController* PC = GetOwningPlayer())
	{
		RiftPlayerState = PC->GetPlayerState<ABasePlayerState>();
		if (RiftPlayerState)
		{
			ASC = RiftPlayerState->GetAbilitySystemComponent();
		}
	}

	if (!ASC)
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().SetTimer(
				InitRetryTimerHandle,
				this,
				&UPlayerHUDWidget::TryInitialize,
				0.2f,
				false
			);
		}
		return;
	}

	BindToAbilitySystem(ASC);
	BindToPlayerState(RiftPlayerState);
	bInitialized = true;
	HandleRespawnStateChanged();
}

void UPlayerHUDWidget::BindToAbilitySystem(UAbilitySystemComponent* ASC)
{
	AbilitySystemComponent = ASC;

	ASC->GetGameplayAttributeValueChangeDelegate(URiftPlayerAttributeSet::GetHealthAttribute()).AddUObject(
		this,
		&UPlayerHUDWidget::HandleHealthChanged
	);
	ASC->GetGameplayAttributeValueChangeDelegate(URiftPlayerAttributeSet::GetMaxHealthAttribute()).AddUObject(
		this,
		&UPlayerHUDWidget::HandleHealthChanged
	);
	ASC->GetGameplayAttributeValueChangeDelegate(URiftPlayerAttributeSet::GetStaminaAttribute()).AddUObject(
		this,
		&UPlayerHUDWidget::HandleStaminaChanged
	);
	ASC->GetGameplayAttributeValueChangeDelegate(URiftPlayerAttributeSet::GetMaxStaminaAttribute()).AddUObject(
		this,
		&UPlayerHUDWidget::HandleStaminaChanged
	);
	ASC->GetGameplayAttributeValueChangeDelegate(URiftResourceAttributeSet::GetUltimateChargeAttribute()).AddUObject(
		this,
		&UPlayerHUDWidget::HandleUltimateChargeChanged
	);
	ASC->GetGameplayAttributeValueChangeDelegate(URiftResourceAttributeSet::GetMaxUltimateChargeAttribute()).AddUObject(
		this,
		&UPlayerHUDWidget::HandleUltimateChargeChanged
	);

	RefreshAll();
}

void UPlayerHUDWidget::BindToPlayerState(ABasePlayerState* PlayerState)
{
	if (!PlayerState || BoundPlayerState.Get() == PlayerState)
	{
		return;
	}

	UnbindFromPlayerState();
	BoundPlayerState = PlayerState;
	PlayerState->OnRespawnStateChanged.AddUniqueDynamic(this, &UPlayerHUDWidget::HandleRespawnStateChanged);
}

void UPlayerHUDWidget::UnbindFromPlayerState()
{
	ABasePlayerState* PlayerState = BoundPlayerState.Get();
	if (!PlayerState)
	{
		BoundPlayerState.Reset();
		return;
	}

	PlayerState->OnRespawnStateChanged.RemoveDynamic(this, &UPlayerHUDWidget::HandleRespawnStateChanged);
	BoundPlayerState.Reset();
}

void UPlayerHUDWidget::RefreshAll()
{
	UAbilitySystemComponent* ASC = AbilitySystemComponent.Get();
	if (!ASC) return;

	OnHealthChanged(
		ASC->GetNumericAttribute(URiftPlayerAttributeSet::GetHealthAttribute()),
		ASC->GetNumericAttribute(URiftPlayerAttributeSet::GetMaxHealthAttribute())
	);
	OnStaminaChanged(
		ASC->GetNumericAttribute(URiftPlayerAttributeSet::GetStaminaAttribute()),
		ASC->GetNumericAttribute(URiftPlayerAttributeSet::GetMaxStaminaAttribute())
	);
	OnUltimateChargeChanged(
		ASC->GetNumericAttribute(URiftResourceAttributeSet::GetUltimateChargeAttribute()),
		ASC->GetNumericAttribute(URiftResourceAttributeSet::GetMaxUltimateChargeAttribute())
	);
}

void UPlayerHUDWidget::HandleRespawnStateChanged()
{
	ABasePlayerState* PlayerState = BoundPlayerState.Get();
	const bool bWaitingForRespawn = PlayerState && PlayerState->IsWaitingForRespawn();

	OnRespawnStateChanged(bWaitingForRespawn);

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	World->GetTimerManager().ClearTimer(RespawnCountdownTimerHandle);

	if (!bWaitingForRespawn)
	{
		OnRespawnCountdownChanged(0.0f);
		return;
	}

	UpdateRespawnCountdown();
	World->GetTimerManager().SetTimer(
		RespawnCountdownTimerHandle,
		this,
		&UPlayerHUDWidget::UpdateRespawnCountdown,
		0.25f,
		true
	);
}

void UPlayerHUDWidget::UpdateRespawnCountdown()
{
	ABasePlayerState* PlayerState = BoundPlayerState.Get();
	const float RemainingSeconds = PlayerState ? PlayerState->GetRespawnRemainingTime() : 0.0f;
	OnRespawnCountdownChanged(RemainingSeconds);

	if (RemainingSeconds > 0.0f)
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(RespawnCountdownTimerHandle);
	}
}

void UPlayerHUDWidget::HandleHealthChanged(const FOnAttributeChangeData&)
{
	UAbilitySystemComponent* ASC = AbilitySystemComponent.Get();
	if (!ASC) return;

	OnHealthChanged(
		ASC->GetNumericAttribute(URiftPlayerAttributeSet::GetHealthAttribute()),
		ASC->GetNumericAttribute(URiftPlayerAttributeSet::GetMaxHealthAttribute())
	);
}

void UPlayerHUDWidget::HandleStaminaChanged(const FOnAttributeChangeData&)
{
	UAbilitySystemComponent* ASC = AbilitySystemComponent.Get();
	if (!ASC) return;

	OnStaminaChanged(
		ASC->GetNumericAttribute(URiftPlayerAttributeSet::GetStaminaAttribute()),
		ASC->GetNumericAttribute(URiftPlayerAttributeSet::GetMaxStaminaAttribute())
	);
}

void UPlayerHUDWidget::HandleUltimateChargeChanged(const FOnAttributeChangeData&)
{
	UAbilitySystemComponent* ASC = AbilitySystemComponent.Get();
	if (!ASC) return;

	OnUltimateChargeChanged(
		ASC->GetNumericAttribute(URiftResourceAttributeSet::GetUltimateChargeAttribute()),
		ASC->GetNumericAttribute(URiftResourceAttributeSet::GetMaxUltimateChargeAttribute())
	);
}
