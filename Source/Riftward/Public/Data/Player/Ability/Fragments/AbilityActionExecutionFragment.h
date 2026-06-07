// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Data/Player/Ability/Fragments/AbilityConfigFragment.h"
#include "AbilityActionExecutionFragment.generated.h"

UENUM(BlueprintType)
enum class EAbilityActionExecutionMode : uint8
{
	ForwardHit	UMETA(DisplayName="Forward Hit"),
	ApplyTimedState	UMETA(DisplayName="Apply Timed State")
};

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class RIFTWARD_API UAbilityActionExecutionFragment : public UAbilityConfigFragment
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Action Execution")
	EAbilityActionExecutionMode ExecutionMode = EAbilityActionExecutionMode::ForwardHit;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Action Execution")
	FGameplayTag ExecutionEventTag;
};
