// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "EnemyAnimationConfig.generated.h"

class UAnimInstance;
class UAnimMontage;
class UAnimSequence;
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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Hit")
	TObjectPtr<UAnimMontage> HitMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Staggered")
	TObjectPtr<UAnimSequence> StaggeredStartSequence;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Staggered")
	TObjectPtr<UAnimSequence> StaggeredLoopSequence;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Staggered")
	TObjectPtr<UAnimSequence> StaggeredEndSequence;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Death")
	TObjectPtr<UAnimMontage> DeathMontage;
};
