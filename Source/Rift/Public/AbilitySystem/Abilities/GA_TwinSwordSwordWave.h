#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "TimerManager.h"
#include "GA_TwinSwordSwordWave.generated.h"

class APlayerCharacter;

UCLASS()
class RIFT_API UGA_TwinSwordSwordWave : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_TwinSwordSwordWave();

	virtual void ActivateAbility(
		FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData
	) override;

	virtual void EndAbility(
		FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled
	) override;

	UFUNCTION(BlueprintCallable, Category="TwinSword|SwordWave")
	void SpawnSwordWaveProjectile();

	UFUNCTION(BlueprintPure, Category="TwinSword|SwordWave")
	int32 GetCurrentCharges() const { return CurrentCharges; }

protected:
	UFUNCTION()
	void HandleMontageCompleted();

	UFUNCTION()
	void HandleMontageBlendOut();

	UFUNCTION()
	void HandleMontageInterrupted();

	UFUNCTION()
	void HandleMontageCancelled();

private:
	void FinishSwordWaveAbility(bool bWasCancelled);
	void HandleRechargeTimer();

	int32 CurrentCharges = INDEX_NONE;
	bool bIsFinishingSwordWave = false;
	bool bHasSpawnedProjectile = false;
	FTimerHandle RechargeTimerHandle;
};
