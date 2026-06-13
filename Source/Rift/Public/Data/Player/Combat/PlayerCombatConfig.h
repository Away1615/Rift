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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Attack")
	TArray<float> PrimaryAttackSectionDamage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Attack")
	TArray<float> PrimaryAttackSectionPoiseDamage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Attack|HitStop")
	float HitStopDuration = 0.05f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Attack|HitStop")
	float HitStopTimeDilation = 0.1f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="TargetAssist")
	float TargetAssistMaxRange = 800.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="TargetAssist")
	float TargetAssistMaxAngleDegrees = 75.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="TargetAssist")
	float TargetAssistAngleWeight = 0.7f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="TargetAssist")
	float TargetAssistDistanceWeight = 0.3f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="TargetAssist")
	float AssistFacingDuration = 0.12f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="TargetAssist")
	float AssistFacingRotationSpeed = 1200.0f;
};
