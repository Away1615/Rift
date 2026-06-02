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
	Signature	= 3 UMETA(DisplayName="Signature"),
	Enhance		= 4 UMETA(DisplayName="Enhance"),
	Ultimate	= 5 UMETA(DisplayName="Ultimate"),
	Interact	= 6 UMETA(DisplayName="Interact"),
	Core		= 7 UMETA(DisplayName="Core")
};
