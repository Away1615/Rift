// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Data/Ability/PlayerAbilitySetConfig.h"
#include "Engine/DataAsset.h"
#include "PlayerClassConfig.generated.h"

class UBaseAbilityConfig;
class UPlayerCommonConfig;
class UBaseGameplayAbility;
class UPlayerAnimationConfig;
class UPlayerWeaponConfig;
/**
 *
 */
UCLASS(BlueprintType)
class RIFTWARD_API UPlayerClassConfig : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Common")
	TObjectPtr<UPlayerCommonConfig> PlayerCommonConfig;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon")
	TObjectPtr<UPlayerWeaponConfig> PlayerWeaponConfig;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Animation")
	TObjectPtr<UPlayerAnimationConfig> PlayerAnimationConfig;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability")
	TObjectPtr<UPlayerAbilitySetConfig> PlayerAbilityConfig;

};
