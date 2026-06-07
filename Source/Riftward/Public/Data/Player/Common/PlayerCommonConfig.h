// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PlayerCommonConfig.generated.h"

/**
 *
 */
UCLASS(BlueprintType)
class RIFTWARD_API UPlayerCommonConfig: public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPlayerCommonConfig();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Attribute|Health")
	float Health = 100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Attribute|Health")
	float MaxHealth = 100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Attribute|Health")
	float HealthRecoverRate = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Attribute|Stamina")
	float Stamina = 100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Attribute|Stamina")
	float MaxStamina = 100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Attribute|Mana")
	float Mana = 100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Attribute|Mana")
	float MaxMana = 100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Attribute|Ultimate")
	float UltimateCharge = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Attribute|Ultimate")
	float MaxUltimateCharge = 100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Movement")
	float MaxAcceleration = 900.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Movement")
	float BrakingDecelerationWalking = 700.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Movement")
	float BrakingFriction = 3;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Movement")
	float BrakingFrictionFactor = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Movement")
	float GroundFriction = 8;

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
};
