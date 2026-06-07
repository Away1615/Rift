// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Data/Player/Ability/Fragments/AbilityConfigFragment.h"
#include "Engine/DataAsset.h"
#include "AbilityDefinitionConfig.generated.h"

class UBaseGameplayAbility;
class UTexture2D;

UCLASS(BlueprintType)
class RIFTWARD_API UAbilityDefinitionConfig : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability")
	FGameplayTag AbilityID;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability")
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability")
	TObjectPtr<UTexture2D> Icon;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability")
	TSubclassOf<UBaseGameplayAbility> AbilityClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Tags")
	FGameplayTagContainer RequiredTags;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Tags")
	FGameplayTagContainer BlockedTags;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Tags")
	FGameplayTag ActiveStateTag;

	UPROPERTY(EditDefaultsOnly, Instanced, BlueprintReadOnly, Category="Fragments")
	TArray<TObjectPtr<UAbilityConfigFragment>> Fragments;

	template <typename FragmentType>
	const FragmentType* FindFragment() const
	{
		for (const TObjectPtr<UAbilityConfigFragment>& Fragment : Fragments)
		{
			if (const FragmentType* TypedFragment = Cast<FragmentType>(Fragment))
			{
				return TypedFragment;
			}
		}
		return nullptr;
	}

	template <typename FragmentType>
	FragmentType* FindMutableFragment()
	{
		for (const TObjectPtr<UAbilityConfigFragment>& Fragment : Fragments)
		{
			if (FragmentType* TypedFragment = Cast<FragmentType>(Fragment))
			{
				return TypedFragment;
			}
		}
		return nullptr;
	}
};
