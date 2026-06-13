// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PlayerCombatConfig.generated.h"

class UAnimMontage;

UCLASS(BlueprintType)
class RIFT_API UPlayerCombatConfig : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPlayerCombatConfig();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Attack")
	TObjectPtr<UAnimMontage> PrimaryAttackMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Attack")
	TArray<FName> PrimaryAttackSections;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Attack")
	float ComboInputBufferDuration = 0.25f;
};
