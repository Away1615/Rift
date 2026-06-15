// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "EnemyAnimationConfig.generated.h"

class UAnimInstance;
class UAnimMontage;
class USkeletalMesh;
class UStaticMesh;

UCLASS(BlueprintType)
class RIFT_API UEnemyAnimationConfig : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UEnemyAnimationConfig();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Appearance")
	TObjectPtr<USkeletalMesh> SkeletalMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Appearance")
	TObjectPtr<UStaticMesh> HeadMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Animation")
	TSubclassOf<UAnimInstance> AnimInstanceClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="HitReact")
	TObjectPtr<UAnimMontage> FlinchMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="HitReact")
	TObjectPtr<UAnimMontage> StaggerMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="HitReact")
	TObjectPtr<UAnimMontage> DeathMontage;
};
