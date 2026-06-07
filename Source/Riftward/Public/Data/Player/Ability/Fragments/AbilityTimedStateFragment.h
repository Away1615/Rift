// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Data/Player/Ability/Fragments/AbilityConfigFragment.h"
#include "GameplayTagContainer.h"
#include "AbilityTimedStateFragment.generated.h"

class UGameplayEffect;

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class RIFTWARD_API UAbilityTimedStateFragment : public UAbilityConfigFragment
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Timed State")
	FGameplayTag ActiveStateTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Timed State", meta=(ClampMin="0.0"))
	float Duration = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Timed State")
	TSubclassOf<UGameplayEffect> StateEffectClass;
};
