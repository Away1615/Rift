// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "EnemyCommonConfig.generated.h"

UCLASS(BlueprintType)
class RIFT_API UEnemyCommonConfig : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Attribute|Health")
	float Health = 100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Attribute|Health")
	float MaxHealth = 100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Attribute|Poise")
	float Poise = 100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Attribute|Poise")
	float MaxPoise = 100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Attribute|Poise")
	float PoiseRegenDelay = 2.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Attribute|Rage")
	float Rage = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Attribute|Rage")
	float MaxRage = 100.0f;
};
