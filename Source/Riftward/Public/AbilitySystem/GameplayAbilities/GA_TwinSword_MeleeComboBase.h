// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/GameplayAbilities/GA_PlayMontage.h"
#include "Data/Player/Ability/AbilityEffectConfig.h"
#include "Data/Player/Ability/Fragments/AbilityMontageSectionsFragment.h"
#include "Data/Player/Input/AbilityInputID.h"
#include "GameplayEffectTypes.h"
#include "GameplayTagContainer.h"
#include "GA_TwinSword_MeleeComboBase.generated.h"

class UAnimMontage;
class APlayerWeapon;
class UAbilityMontageSectionsFragment;
class UAbilityEffectListFragment;

UCLASS(Abstract)
class RIFTWARD_API UGA_TwinSword_MeleeComboBase : public UMontageGameplayAbility
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

	virtual FGameplayTag GetAbilityActiveStateTag() const override;
	virtual bool ShouldCommitComboAbility() const;
	virtual bool OnAbilityMontageCancelled() override;
	virtual bool OnAbilityMontageInterrupted() override;
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
		TSet<TWeakObjectPtr<AActor>> HitActors;
		bool bTraceActive = false;
	};

	UPROPERTY()
	TObjectPtr<UAnimMontage> ActiveMontage;

	UPROPERTY()
	TObjectPtr<UAnimMontage> ComboMontage;

	TArray<FAbilityMontageSection> ActiveComboSections;
	TArray<FWeaponTraceState> WeaponTraceStates;
	FActiveGameplayEffectHandle ComboActiveEffectHandle;
	int32 ActiveSectionIndex = INDEX_NONE;
	EAbilityInputID BufferedAttackInput = EAbilityInputID::None;
	bool bCanConsumeBufferedInput = false;

	void WaitForCurrentSectionChainPoint();
	void WaitForComboInput();
	void WaitForComboInputEvents();
	void WaitForWeaponTraceEvents();
	void BufferInput(EAbilityInputID InputID);
	void ConsumeBufferedInputAtChainPoint();
	void TryAdvanceComboSection();
	bool TryActivateBufferedComboAbility(EAbilityInputID InputID);
	bool TryFindAbilityHandleForInput(EAbilityInputID InputID, FGameplayAbilitySpecHandle& OutHandle) const;
	EAbilityInputID GetCurrentAbilityInputID() const;
	bool PlayComboSection(int32 SectionIndex);
	UAnimMontage* ResolveSectionMontage(const FAbilityMontageSection& Section) const;
	FName ResolveSectionName(const FAbilityMontageSection& Section) const;
	bool IsCurrentSectionWaitingForChainPoint() const;
	const UAbilityMontageSectionsFragment* GetSectionsFragment() const;
	void SelectActiveComboSections(const UAbilityMontageSectionsFragment* SectionsFragment);
	void InitWeaponTraceStates();
	// WeaponIndex: INDEX_NONE(-1)=两把剑, 0=右剑, 1=左剑
	void BeginWeaponTrace(int32 WeaponIndex);
	void PerformWeaponTrace(int32 WeaponIndex);
	void EndWeaponTrace(int32 WeaponIndex);
	void ResetWeaponTrace();
	void ApplyDamageToHitActor(AActor* HitActor, const FHitResult& Hit);
	void ApplyHitEffects(AActor* HitActor, const FHitResult& Hit, const TArray<FAbilityEffectConfig>& StepEffects) const;
	void ApplyHealthEffect(AActor* TargetActor, const FHitResult& Hit, float Magnitude) const;
	void ApplySwordWave(const FAbilityEffectConfig& Effect) const;
	void ApplyCooldownReduction(const FAbilityEffectConfig& Effect) const;
	void CollectAbilityEffects(TArray<FAbilityEffectConfig>& OutEffects) const;
	bool DoesEffectPassCondition(const FAbilityEffectConfig& Effect) const;
	float GetActiveDamageMultiplier() const;
	float GetActiveAttackSpeedMultiplier() const;
	float GetActiveAttackRangeMultiplier() const;

	UFUNCTION()
	void HandleComboChainPoint(FGameplayEventData Payload);

	UFUNCTION()
	void HandleComboInputPressed(float TimeWaited);

	UFUNCTION()
	void HandlePrimaryComboInputEvent(FGameplayEventData Payload);

	UFUNCTION()
	void HandleSecondaryComboInputEvent(FGameplayEventData Payload);

	UFUNCTION()
	void HandleWeaponTraceBegin(FGameplayEventData Payload);

	UFUNCTION()
	void HandleWeaponTraceTick(FGameplayEventData Payload);

	UFUNCTION()
	void HandleWeaponTraceEnd(FGameplayEventData Payload);

};
