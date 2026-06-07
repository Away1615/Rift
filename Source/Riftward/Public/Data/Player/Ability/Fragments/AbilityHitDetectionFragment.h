// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Data/Player/Ability/Fragments/AbilityConfigFragment.h"
#include "AbilityHitDetectionFragment.generated.h"

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class RIFTWARD_API UAbilityHitDetectionFragment : public UAbilityConfigFragment
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Hit", meta=(ClampMin="0.0"))
	float Range = 450.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Hit", meta=(ClampMin="1.0"))
	float Radius = 80.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Hit", meta=(ClampMin="0.01"))
	float EnhancedRadiusMultiplier = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Hit")
	bool bDrawDebug = false;
};
