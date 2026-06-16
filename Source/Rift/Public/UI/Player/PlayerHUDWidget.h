// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TimerManager.h"
#include "PlayerHUDWidget.generated.h"

class UAbilitySystemComponent;
class ABasePlayerState;
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

	UFUNCTION(BlueprintImplementableEvent, Category="HUD|Respawn")
	void OnRespawnStateChanged(bool bWaitingForRespawn);

	UFUNCTION(BlueprintImplementableEvent, Category="HUD|Respawn")
	void OnRespawnCountdownChanged(float RemainingSeconds);

private:
	void TryInitialize();
	void BindToAbilitySystem(UAbilitySystemComponent* ASC);
	void BindToPlayerState(ABasePlayerState* PlayerState);
	void UnbindFromPlayerState();
	void RefreshAll();
	void HandleHealthChanged(const FOnAttributeChangeData& Data);
	void HandleStaminaChanged(const FOnAttributeChangeData& Data);
	void HandleUltimateChargeChanged(const FOnAttributeChangeData& Data);

	UFUNCTION()
	void HandleRespawnStateChanged();

	void UpdateRespawnCountdown();

	TWeakObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;
	TWeakObjectPtr<ABasePlayerState> BoundPlayerState;
	FTimerHandle InitRetryTimerHandle;
	FTimerHandle RespawnCountdownTimerHandle;
	bool bInitialized = false;
};
