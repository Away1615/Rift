// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/GameplayAbilities/BaseGameplayAbility.h"
#include "GA_PlayMontage.generated.h"

class UAnimMontage;

UCLASS(Abstract)
class RIFTWARD_API UMontageGameplayAbility : public UBaseGameplayAbility
{
	GENERATED_BODY()

protected:
	virtual bool OnAbilityMontageCompleted();
	virtual void OnAbilityMontageBlendedIn();
	virtual void OnAbilityMontageBlendOut();
	virtual bool OnAbilityMontageCancelled();
	virtual bool OnAbilityMontageInterrupted();

	// ~ Create Montage Task and try to play it
	bool TryPlayMontage(
		FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		UAnimMontage* MontageToPlay,
		float PlayRate,
		FName StartSection,
		bool bEndAbilityOnFinish);

private:
	bool bEndAbilityOnCurrentMontage = false;

	UFUNCTION()
	void HandleMontageCompleted();

	UFUNCTION()
	void HandleMontageBlendedIn();

	UFUNCTION()
	void HandleMontageBlendOut();

	UFUNCTION()
	void HandleMontageCancelled();

	UFUNCTION()
	void HandleMontageInterrupted();
};

UCLASS(Blueprintable)
class RIFTWARD_API UGA_PlayMontage : public UMontageGameplayAbility
{
	GENERATED_BODY()

protected:
	virtual void ActivateAbility(
		FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;
};
