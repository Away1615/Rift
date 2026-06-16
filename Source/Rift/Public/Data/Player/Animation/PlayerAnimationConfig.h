// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PlayerAnimationConfig.generated.h"

class UAnimInstance;
class UAnimMontage;
class USkeletalMesh;

/**
 *
 */
UCLASS(BlueprintType)
class RIFT_API UPlayerAnimationConfig: public UPrimaryDataAsset
{
	GENERATED_BODY()
public:
	UPlayerAnimationConfig();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Appearance")
	TObjectPtr<USkeletalMesh> SkeletalMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Animation")
	TSubclassOf<UAnimInstance> AnimInstanceClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Hit")
	TObjectPtr<UAnimMontage> LightHitMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Hit")
	TObjectPtr<UAnimMontage> HeavyHitMontage;
};
