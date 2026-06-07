// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Data/Player/Ability/Fragments/AbilityConfigFragment.h"
#include "AbilityResourceCostFragment.generated.h"

class UGameplayEffect;

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class RIFTWARD_API UAbilityResourceCostFragment : public UAbilityConfigFragment
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Cost", meta=(ClampMin="0.0"))
	float Stamina = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Cost", meta=(ClampMin="0.0"))
	float Mana = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Cost")
	TSubclassOf<UGameplayEffect> CostEffectClass;
};
