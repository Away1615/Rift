// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Data/Player/Ability/BaseAbilityConfig.h"
#include "TwinSwordComboAbilityConfig.generated.h"

class UGameplayEffect;
class UAnimMontage;

UCLASS(Abstract, BlueprintType)
class RIFTWARD_API UTwinSwordComboAbilityConfig : public UBaseAbilityConfig
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Animation")
	TObjectPtr<UAnimMontage> AttackMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Animation")
	TObjectPtr<UAnimMontage> SuperAttackMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combo")
	TArray<FName> ComboSections;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Cost", meta=(ClampMin="0.0"))
	float StaminaCost = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Cost")
	TSubclassOf<UGameplayEffect> StaminaCostEffectClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="HitCheck")
	float HitRange = 180.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="HitCheck")
	float HitRadius = 80.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="HitCheck")
	bool bDrawDebugHitCheck = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Damage")
	TSubclassOf<UGameplayEffect> InstantDamageEffectClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Damage")
	TArray<float> ComboDamages;

};
