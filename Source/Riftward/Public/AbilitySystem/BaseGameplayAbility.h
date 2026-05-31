// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "Input/PlayerInputID.h"
#include "BaseGameplayAbility.generated.h"

/**
 * 
 */
UCLASS()
class RIFTWARD_API UBaseGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input")
	EAbilityInputID AbilityInputID = EAbilityInputID::None;
};
