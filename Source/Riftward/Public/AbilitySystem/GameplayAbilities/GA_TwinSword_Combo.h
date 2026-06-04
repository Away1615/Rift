// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/GameplayAbilities/BaseGameplayAbility.h"
#include "GameplayEffectTypes.h"
#include "GameplayTagContainer.h"
#include "GA_TwinSword_Combo.generated.h"

class UAnimMontage;
class UTwinSwordComboAbilityConfig;

UCLASS(Abstract)
class RIFTWARD_API UGA_TwinSword_Combo : public UBaseGameplayAbility
{
	GENERATED_BODY()

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

	virtual FGameplayTag GetComboWindowEventTag() const;
	virtual FGameplayTag GetAbilityActiveStateTag() const override;
	virtual bool ShouldCommitComboAbility() const;

private:
	UPROPERTY()
	TObjectPtr<UAnimMontage> ActiveMontage;

	TArray<FName> ActiveComboSections;
	bool bHasBufferedInput = false;
	bool bCanConsumeBufferedInput = false;
	bool bAddedComboActiveTag = false;

	const UTwinSwordComboAbilityConfig* GetComboConfig() const;
	void ResetComboSectionLinks() const;
	void WaitForComboWindow();
	void WaitForComboInput();
	void BufferInput();
	void TryConsumeBufferedInput();
	void JumpToNextComboSection();
	int32 GetCurrentComboSectionIndex() const;

	UFUNCTION()
	void HandleComboWindow(FGameplayEventData Payload);

	UFUNCTION()
	void HandleComboInputPressed(float TimeWaited);

	UFUNCTION()
	void HandleMontageCompleted();

	UFUNCTION()
	void HandleMontageCancelled();

	UFUNCTION()
	void HandleMontageInterrupted();
};
