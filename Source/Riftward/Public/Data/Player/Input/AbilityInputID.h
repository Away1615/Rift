// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilityInputID.generated.h"

UENUM(BlueprintType)
enum class EAbilityInputID : uint8
{
	None		= 0 UMETA(DisplayName="None"),

	Primary		= 1 UMETA(DisplayName="Primary"),
	Secondary	= 2 UMETA(DisplayName="Secondary"),
	PrimaryHeavy	= 3 UMETA(DisplayName="Primary Heavy"),
	SecondaryHeavy	= 4 UMETA(DisplayName="Secondary Heavy"),
	Enhance		= 5 UMETA(DisplayName="Enhance"),
	Ultimate	= 6 UMETA(DisplayName="Ultimate"),
	Interact	= 7 UMETA(DisplayName="Interact"),
	Core		= 8 UMETA(DisplayName="Core"),
	Special		= 9 UMETA(DisplayName="Special"),
	Signature	= 10 UMETA(Hidden, DisplayName="Deprecated Signature")
};
