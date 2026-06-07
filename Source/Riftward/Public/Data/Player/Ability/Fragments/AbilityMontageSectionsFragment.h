// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Data/Player/Ability/AbilityEffectConfig.h"
#include "Data/Player/Ability/Fragments/AbilityConfigFragment.h"
#include "GameplayTagContainer.h"
#include "AbilityMontageSectionsFragment.generated.h"

class UAnimMontage;

USTRUCT(BlueprintType)
struct FAbilityMontageSection
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Section")
	FName SectionName = NAME_None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Section|Legacy", meta=(AdvancedDisplay, DisplayName="Montage Override"))
	TObjectPtr<UAnimMontage> Montage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Section", meta=(ClampMin="0.01"))
	float PlayRate = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Section")
	FGameplayTag ChainPointEventTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Section")
	TArray<FAbilityEffectConfig> Effects;
};

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class RIFTWARD_API UAbilityMontageSectionsFragment : public UAbilityConfigFragment
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Montage")
	TObjectPtr<UAnimMontage> Montage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Sections")
	TArray<FAbilityMontageSection> Sections;
};
