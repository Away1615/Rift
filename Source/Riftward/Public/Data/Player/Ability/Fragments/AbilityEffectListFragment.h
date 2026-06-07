// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Data/Player/Ability/AbilityEffectConfig.h"
#include "Data/Player/Ability/Fragments/AbilityConfigFragment.h"
#include "AbilityEffectListFragment.generated.h"

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class RIFTWARD_API UAbilityEffectListFragment : public UAbilityConfigFragment
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Effects")
	TArray<FAbilityEffectConfig> Effects;
};
