// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "Data/Player/Input/AbilityInputID.h"
#include "BaseGameplayAbility.generated.h"

/**
 *
 */
UCLASS()
class RIFTWARD_API UBaseGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input")
	EAbilityInputID AbilityInputID = EAbilityInputID::None;

	virtual bool CanActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayTagContainer* SourceTags = nullptr,
		const FGameplayTagContainer* TargetTags = nullptr,
		FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

protected:
	virtual FGameplayTag GetAbilityActiveStateTag() const;
};
