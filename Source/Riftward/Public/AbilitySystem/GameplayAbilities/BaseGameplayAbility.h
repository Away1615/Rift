// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "Data/Player/Input/AbilityInputID.h"
#include "BaseGameplayAbility.generated.h"

class UPlayerAbilitySetConfig;
struct FPlayerAbilityEntry;

/**
 *
 */
UCLASS()
class RIFTWARD_API UBaseGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input", meta=(DeprecatedProperty))
	EAbilityInputID AbilityInputID = EAbilityInputID::None;

	virtual bool CanActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayTagContainer* SourceTags = nullptr,
		const FGameplayTagContainer* TargetTags = nullptr,
		FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

protected:
	virtual bool CheckCost(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

	virtual void ApplyCost(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo) const override;

	virtual FGameplayTag GetAbilityActiveStateTag() const;

	const FPlayerAbilityEntry* GetAbilityEntry() const;
	const FPlayerAbilityEntry* GetAbilityEntryFromSpec(
		FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo) const;
	const UPlayerAbilitySetConfig* GetAbilitySetConfigFromSpec(
		FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo) const;
};
