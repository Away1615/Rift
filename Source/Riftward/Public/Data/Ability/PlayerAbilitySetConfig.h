// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PlayerAbilitySetConfig.generated.h"

class UBaseGameplayAbility;
class UBaseAbilityConfig;
/**
 *
 */
UCLASS(BlueprintType)
class RIFTWARD_API UPlayerAbilitySetConfig : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPlayerAbilitySetConfig();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability")
	TArray<TSubclassOf<UBaseGameplayAbility>> PassiveAbilities;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability")
	TObjectPtr<UBaseAbilityConfig> CoreAbilityConfig;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability")
	TObjectPtr<UBaseAbilityConfig> PrimaryAbilityConfig;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability")
	TObjectPtr<UBaseAbilityConfig> SecondaryAbilityConfig;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability")
	TObjectPtr<UBaseAbilityConfig> SignatureAbilityConfig;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability")
	TObjectPtr<UBaseAbilityConfig> EnhanceAbilityConfig;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability")
	TObjectPtr<UBaseAbilityConfig> UltimateAbilityConfig;

};
