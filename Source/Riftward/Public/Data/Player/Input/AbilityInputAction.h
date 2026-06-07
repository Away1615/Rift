// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilityInputID.h"
#include "InputTriggers.h"
#include "UObject/Object.h"
#include "AbilityInputAction.generated.h"

class UInputAction;
/**
 *
 */
USTRUCT(BlueprintType)
struct FAbilityInputAction
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UInputAction> InputAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	EAbilityInputID InputID = EAbilityInputID::None;

	UPROPERTY(EditAnywhere)
	ETriggerEvent TriggerEvent = ETriggerEvent::Started;
};
