// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/GameplayAbilities/BaseGameplayAbility.h"
#include "GA_TwinSword_Core.generated.h"

class UAnimMontage;
class APlayerCharacter;
class UTwinSwordCoreAbilityConfig;

/**
 *
 */
UCLASS()
class RIFTWARD_API UGA_TwinSword_Core : public UBaseGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_TwinSword_Core();

	virtual void InputReleased(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo) override;

protected:

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled) override;

	 virtual FGameplayTag GetAbilityActiveStateTag() const override;
private:
	UPROPERTY()
	TObjectPtr<UAnimMontage> ActiveMontage;

	bool bPlayRecover = false;
	bool bAddedDodgeActiveTag = false;

	UFUNCTION()
	void HandleDodgeFinished(FGameplayEventData Payload);

	UFUNCTION()
	void HandleMontageCompleted();

	UFUNCTION()
	void HandleMontageCancelled();

	UFUNCTION()
	void HandleMontageInterrupted();
};
