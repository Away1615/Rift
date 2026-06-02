// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "BaseAbilityConfig.generated.h"

class UBaseGameplayAbility;
/**
 *
 */
UCLASS(Abstract, BlueprintType)
class RIFTWARD_API UBaseAbilityConfig : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability")
	TSubclassOf<UBaseGameplayAbility> AbilityClass;
};
