// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/GameplayAbilities/BaseGameplayAbility.h"
#include "GameplayEffectTypes.h"
#include "GameplayTagContainer.h"
#include "GA_TwinSword_Combo.generated.h"

class UAnimMontage;
class UTwinSwordComboAbilityConfig;
class APlayerWeapon;

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
	virtual bool CheckCost(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void ApplyCost(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo) const override;

private:
	struct FWeaponTraceState
	{
		TWeakObjectPtr<APlayerWeapon> Weapon;
		FVector PreviousStart = FVector::ZeroVector;
		FVector PreviousEnd = FVector::ZeroVector;
	};

	UPROPERTY()
	TObjectPtr<UAnimMontage> ActiveMontage;

	TArray<FName> ActiveComboSections;
	TArray<FWeaponTraceState> WeaponTraceStates;
	TSet<TWeakObjectPtr<AActor>> HitActorsThisTraceWindow;
	bool bHasBufferedInput = false;
	bool bCanConsumeBufferedInput = false;
	bool bAddedComboActiveTag = false;
	bool bWeaponTraceActive = false;

	const UTwinSwordComboAbilityConfig* GetComboConfig() const;
	const UTwinSwordComboAbilityConfig* GetComboConfigFromSpec(
		FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo) const;
	void ResetComboSectionLinks() const;
	void WaitForComboWindow();
	void WaitForComboInput();
	void WaitForWeaponTraceEvents();
	void BufferInput();
	void TryConsumeBufferedInput();
	void JumpToNextComboSection();
	int32 GetCurrentComboSectionIndex() const;
	void BeginWeaponTrace();
	void PerformWeaponTrace();
	void EndWeaponTrace();
	void ResetWeaponTrace();
	void ApplyDamageToHitActor(AActor* HitActor, const FHitResult& Hit);

	UFUNCTION()
	void HandleComboWindow(FGameplayEventData Payload);

	UFUNCTION()
	void HandleComboInputPressed(float TimeWaited);

	UFUNCTION()
	void HandleWeaponTraceBegin(FGameplayEventData Payload);

	UFUNCTION()
	void HandleWeaponTraceTick(FGameplayEventData Payload);

	UFUNCTION()
	void HandleWeaponTraceEnd(FGameplayEventData Payload);

	UFUNCTION()
	void HandleMontageCompleted();

	UFUNCTION()
	void HandleMontageCancelled();

	UFUNCTION()
	void HandleMontageInterrupted();
};
