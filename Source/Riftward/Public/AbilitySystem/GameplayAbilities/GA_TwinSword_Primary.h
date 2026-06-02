// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/GameplayAbilities/BaseGameplayAbility.h"
#include "GameplayEffectTypes.h"
#include "GA_TwinSword_Primary.generated.h"

class UAnimMontage;

/**
 *
 */
UCLASS()
class RIFTWARD_API UGA_TwinSword_Primary : public UBaseGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_TwinSword_Primary();

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

private:
	UPROPERTY()
	TObjectPtr<UAnimMontage> ActiveMontage;

	TArray<FName> ActiveComboSections;
	bool bHasBufferedPrimaryInput = false;
	bool bCanConsumeBufferedPrimaryInput = false;

	void ResetComboSectionLinks() const;
	void WaitForNextPrimaryInput();
	void WaitForComboWindow();
	void BufferPrimaryInput();
	void TryConsumeBufferedInput();
	void JumpToNextComboSection();
	int32 GetCurrentComboSectionIndex() const;

	UFUNCTION()
	void HandleInputPressed(float TimeWaited);

	UFUNCTION()
	void HandleComboWindow(FGameplayEventData Payload);

	UFUNCTION()
	void HandleMontageCompleted();

	UFUNCTION()
	void HandleMontageCancelled();

	UFUNCTION()
	void HandleMontageInterrupted();
};
