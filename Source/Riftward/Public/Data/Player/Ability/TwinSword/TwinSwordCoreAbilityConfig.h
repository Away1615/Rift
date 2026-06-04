// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Data/Player/Ability/BaseAbilityConfig.h"
#include "TwinSwordCoreAbilityConfig.generated.h"

class UAnimMontage;

/**
 *
 */
UCLASS(BlueprintType)
class RIFTWARD_API UTwinSwordCoreAbilityConfig : public UBaseAbilityConfig
{
	GENERATED_BODY()

public:
	UTwinSwordCoreAbilityConfig();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Animation|Dodge")
	TObjectPtr<UAnimMontage> DodgeMontage;
};
