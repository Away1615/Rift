// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Combat/RiftWeaponTypes.h"
#include "Engine/DataAsset.h"
#include "PlayerClassConfig.generated.h"

class UAnimInstance;
class UAnimMontage;
class URiftAbilityConfig;
class UStaticMesh;

UCLASS(BlueprintType)
class RIFT_API UPlayerClassConfig : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Attribute")
	float Health = 100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Attribute")
	float MaxHealth = 100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Attribute")
	float UltimateCharge = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Attribute")
	float MaxUltimateCharge = 100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Attribute")
	float RunSpeed = 600.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Animation")
	TSubclassOf<UAnimInstance> AnimClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Animation")
	TObjectPtr<UAnimMontage> LightHitMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Animation")
	TObjectPtr<UAnimMontage> HeavyHitMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Animation")
	TObjectPtr<UAnimMontage> DeathMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon")
	TMap<ERiftWeaponSlot, TObjectPtr<UStaticMesh>> Weapons;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability")
	TArray<TObjectPtr<URiftAbilityConfig>> AbilityConfigs;
};
