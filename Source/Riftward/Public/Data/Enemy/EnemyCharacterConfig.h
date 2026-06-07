// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "EnemyCharacterConfig.generated.h"

class UEnemyCommonConfig;

UCLASS(BlueprintType)
class RIFTWARD_API UEnemyCharacterConfig : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Common")
	TObjectPtr<UEnemyCommonConfig> EnemyCommonConfig;
};
