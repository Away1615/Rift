// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Data/Ability/BaseAbilityConfig.h"
#include "TwinSwordPrimaryAbilityConfig.generated.h"

/**
 *
 */
UCLASS(BlueprintType)
class RIFTWARD_API UTwinSwordPrimaryAbilityConfig : public UBaseAbilityConfig
{
	GENERATED_BODY()
public:
	UTwinSwordPrimaryAbilityConfig();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Animation")
	TObjectPtr<UAnimMontage> AttackMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combo")
	TArray<FName> ComboSections;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="HitCheck")
	float HitRange = 180.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="HitCheck")
	float HitRadius = 80.0f;

};
