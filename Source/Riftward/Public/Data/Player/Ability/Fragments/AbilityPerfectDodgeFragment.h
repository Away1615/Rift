// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Data/Player/Ability/Fragments/AbilityConfigFragment.h"
#include "GameplayTagContainer.h"
#include "AbilityPerfectDodgeFragment.generated.h"

class UAnimMontage;
class UParticleSystem;
class USoundBase;

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class RIFTWARD_API UAbilityPerfectDodgeFragment : public UAbilityConfigFragment
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Perfect Dodge|Reward")
	FGameplayTag PerfectSuccessStateTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Perfect Dodge|Reward", meta=(ClampMin="0.0"))
	float PerfectSuccessStateDuration = 5.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Perfect Dodge")
	TObjectPtr<UAnimMontage> PerfectSuccessMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Perfect Dodge")
	FName PerfectSuccessSection = NAME_None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Perfect Dodge", meta=(ClampMin="0.01"))
	float PerfectSuccessPlayRate = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Perfect Dodge|Cue")
	TObjectPtr<UParticleSystem> PerfectSuccessEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Perfect Dodge|Cue")
	TObjectPtr<USoundBase> PerfectSuccessSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Perfect Dodge|Cue")
	FName CueSocketName = NAME_None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Perfect Dodge|Cue")
	FVector CueLocationOffset = FVector::ZeroVector;
};
