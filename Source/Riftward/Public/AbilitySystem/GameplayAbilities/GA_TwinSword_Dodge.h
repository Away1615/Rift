// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/GameplayAbilities/GA_PlayMontage.h"
#include "GameplayEffectTypes.h"
#include "GA_TwinSword_Dodge.generated.h"

class UAnimMontage;
class APlayerCharacter;
class UAbilityPerfectDodgeFragment;

UCLASS()
class RIFTWARD_API UGA_TwinSword_Dodge : public UMontageGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_TwinSword_Dodge();

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
	virtual bool OnAbilityMontageCancelled() override;
	virtual bool OnAbilityMontageInterrupted() override;

private:
	UPROPERTY()
	TObjectPtr<UAnimMontage> ActiveMontage;

	FActiveGameplayEffectHandle DodgeActiveEffectHandle;
	bool bPlayRecover = false;
	bool bChangingToPerfectDodgeMontage = false;

	UFUNCTION()
	void HandleDodgeFinished(FGameplayEventData Payload);

	UFUNCTION()
	void HandlePerfectDodgeSuccess(FGameplayEventData Payload);

	void PlayPerfectDodgeFeedback(const UAbilityPerfectDodgeFragment* PerfectDodgeFragment);
	void PlayPerfectDodgeMontage(const UAbilityPerfectDodgeFragment* PerfectDodgeFragment);
};
