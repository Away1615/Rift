// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "Engine/DataAsset.h"
#include "PlayerClassConfig.generated.h"

class UPlayerCommonConfig;
class UPlayerAnimationConfig;
class UPlayerWeaponConfig;
/**
 *
 */
UCLASS(BlueprintType)
class RIFT_API UPlayerClassConfig : public UPrimaryDataAsset
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
	TArray<TSubclassOf<UGameplayAbility>> GrantedAbilities;

};
