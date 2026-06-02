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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Movement")
	float WalkSpeed = 200.0f;

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
