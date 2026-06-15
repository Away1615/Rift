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
		ASC->GetGameplayAttributeValueChangeDelegate(URiftResourceAttributeSet::GetSwordIntentAttribute()).RemoveAll(this);
		ASC->GetGameplayAttributeValueChangeDelegate(URiftResourceAttributeSet::GetMaxSwordIntentAttribute()).RemoveAll(this);
		ASC->GetGameplayAttributeValueChangeDelegate(URiftResourceAttributeSet::GetUltimateChargeAttribute()).RemoveAll(this);
		ASC->GetGameplayAttributeValueChangeDelegate(URiftResourceAttributeSet::GetMaxUltimateChargeAttribute()).RemoveAll(this);
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(InitRetryTimerHandle);
	}

	Super::NativeDestruct();
}

void UPlayerHUDWidget::TryInitialize()
{
	if (bInitialized) return;

	UAbilitySystemComponent* ASC = nullptr;
	if (APlayerController* PC = GetOwningPlayer())
	{
		if (ABasePlayerState* PS = PC->GetPlayerState<ABasePlayerState>())
		{
			ASC = PS->GetAbilitySystemComponent();
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
	bInitialized = true;
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
	ASC->GetGameplayAttributeValueChangeDelegate(URiftResourceAttributeSet::GetSwordIntentAttribute()).AddUObject(
		this,
		&UPlayerHUDWidget::HandleSwordIntentChanged
	);
	ASC->GetGameplayAttributeValueChangeDelegate(URiftResourceAttributeSet::GetMaxSwordIntentAttribute()).AddUObject(
		this,
		&UPlayerHUDWidget::HandleSwordIntentChanged
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
	OnSwordIntentChanged(
		ASC->GetNumericAttribute(URiftResourceAttributeSet::GetSwordIntentAttribute()),
		ASC->GetNumericAttribute(URiftResourceAttributeSet::GetMaxSwordIntentAttribute())
	);
	OnUltimateChargeChanged(
		ASC->GetNumericAttribute(URiftResourceAttributeSet::GetUltimateChargeAttribute()),
		ASC->GetNumericAttribute(URiftResourceAttributeSet::GetMaxUltimateChargeAttribute())
	);
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

void UPlayerHUDWidget::HandleSwordIntentChanged(const FOnAttributeChangeData&)
{
	UAbilitySystemComponent* ASC = AbilitySystemComponent.Get();
	if (!ASC) return;

	OnSwordIntentChanged(
		ASC->GetNumericAttribute(URiftResourceAttributeSet::GetSwordIntentAttribute()),
		ASC->GetNumericAttribute(URiftResourceAttributeSet::GetMaxSwordIntentAttribute())
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
