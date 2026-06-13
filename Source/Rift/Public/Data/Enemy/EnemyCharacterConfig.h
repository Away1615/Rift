// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "EnemyCharacterConfig.generated.h"

class UEnemyAnimationConfig;
class UEnemyCommonConfig;

UCLASS(BlueprintType)
class RIFT_API UEnemyCharacterConfig : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Common")
	TObjectPtr<UEnemyCommonConfig> EnemyCommonConfig;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Animation")
	TObjectPtr<UEnemyAnimationConfig> EnemyAnimationConfig;
};
