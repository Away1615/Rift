// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PlayerCombatConfig.generated.h"

class UAnimMontage;
class UCameraShakeBase;
class URiftComboGraph;

USTRUCT(BlueprintType)
struct FRiftHeavyAttackConfig
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UAnimMontage> Montage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float Damage = 30.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float PoiseDamage = 35.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float StaminaCost = 25.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float UltimateChargeOnHit = 4.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UCameraShakeBase> CameraShake;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FVector2D CameraShakeDir = FVector2D(0.0f, -1.0f);
};

UCLASS(BlueprintType)
class RIFT_API UPlayerCombatConfig : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPlayerCombatConfig();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Attack")
	TObjectPtr<URiftComboGraph> ComboGraph;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Attack")
	float ComboInputBufferDuration = 0.25f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Attack|Heavy")
	FRiftHeavyAttackConfig PrimaryHeavy;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Attack|Heavy")
	FRiftHeavyAttackConfig SecondaryHeavy;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Attack|HitStop")
	float HitStopDuration = 0.05f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Attack|HitStop")
	float HitStopTimeDilation = 0.1f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Dodge")
	TObjectPtr<UAnimMontage> DodgeMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Dodge")
	float DodgeStaminaCost = 25.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Dodge")
	float PerfectDodgeWindowDuration = 0.2f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Dodge|Perfect")
	float PerfectDodgeUltimateChargeReward = 8.0f;

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
