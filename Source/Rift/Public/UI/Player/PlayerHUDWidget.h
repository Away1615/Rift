// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TimerManager.h"
#include "PlayerHUDWidget.generated.h"

class UAbilitySystemComponent;
struct FOnAttributeChangeData;

/**
 * 
 */
UCLASS()
class RIFT_API UPlayerHUDWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION(BlueprintImplementableEvent, Category="HUD")
	void OnHealthChanged(float Current, float Max);

	UFUNCTION(BlueprintImplementableEvent, Category="HUD")
	void OnStaminaChanged(float Current, float Max);

	UFUNCTION(BlueprintImplementableEvent, Category="HUD")
	void OnUltimateChargeChanged(float Current, float Max);

private:
	void TryInitialize();
	void BindToAbilitySystem(UAbilitySystemComponent* ASC);
	void RefreshAll();
	void HandleHealthChanged(const FOnAttributeChangeData& Data);
	void HandleStaminaChanged(const FOnAttributeChangeData& Data);
	void HandleUltimateChargeChanged(const FOnAttributeChangeData& Data);

	TWeakObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;
	FTimerHandle InitRetryTimerHandle;
	bool bInitialized = false;
};
