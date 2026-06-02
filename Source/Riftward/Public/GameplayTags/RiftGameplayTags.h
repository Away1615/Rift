// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

struct FRiftGameplayTags
{
public:
	static const FRiftGameplayTags& Get();

	static void InitializeNativeTags();

	// ======================
	// TwinSword
	// ======================
	/* Event */
	FGameplayTag Event_Ability_TwinSword_Core_DodgeFinished;

	FGameplayTag Event_Ability_TwinSword_Primary_ComboWindow;

	FGameplayTag Event_Ability_TwinSword_Primary_HitCheck;

	/* State */
	FGameplayTag State_Ability_TwinSword_Core_PerfectDodgeWindow;

private:
	static FRiftGameplayTags GameplayTags;
};
