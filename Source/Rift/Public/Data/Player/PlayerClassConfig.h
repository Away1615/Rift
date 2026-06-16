// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "Combat/RiftWeaponTypes.h"
#include "Engine/DataAsset.h"
#include "PlayerClassConfig.generated.h"

class UPlayerAnimationConfig;
class UPlayerCombatConfig;
class UStaticMesh;

/**
 *
 */
UCLASS(BlueprintType)
class RIFT_API UPlayerClassConfig : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Attribute|Health")
	float Health = 100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Attribute|Health")
	float MaxHealth = 100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Attribute|Stamina")
	float Stamina = 100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Attribute|Stamina")
	float MaxStamina = 100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Attribute|Poise")
	float Poise = 100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Attribute|Poise")
	float MaxPoise = 100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Attribute|Stamina")
	float StaminaRegenRate = 20.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Attribute|Ultimate")
	float UltimateCharge = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Attribute|Ultimate")
	float MaxUltimateCharge = 100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Movement")
	float MaxAcceleration = 900.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Movement")
	float BrakingDecelerationWalking = 700.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Movement")
	float BrakingFriction = 3.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Movement")
	float BrakingFrictionFactor = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Movement")
	float GroundFriction = 8.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Movement")
	float RunSpeed = 600.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Turn")
	float MinTurnRate = 200.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Turn")
	float MaxTurnRate = 1000.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Turn")
	float MinTurnRateInterpSpeed = 3.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Turn")
	float MaxTurnRateInterpSpeed = 18.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon")
	TMap<ERiftWeaponSlot, TObjectPtr<UStaticMesh>> Weapons;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Animation")
	TObjectPtr<UPlayerAnimationConfig> PlayerAnimationConfig;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat")
	TObjectPtr<UPlayerCombatConfig> PlayerCombatConfig;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability")
	TArray<TSubclassOf<UGameplayAbility>> GrantedAbilities;

};
