// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

struct FRiftGameplayTags
{
public:
	static const FRiftGameplayTags& Get();

	static void InitializeNativeTags();

	FGameplayTag State_Movement_Grounded;
	FGameplayTag State_Movement_Airborne;
	FGameplayTag State_Movement_Climbing;
	FGameplayTag State_Movement_Blocked;

	// Action State
	FGameplayTag State_Action_Sprinting;
	FGameplayTag State_Action_CoreActive;

	// Core Mechanic State
	FGameplayTag State_Core_PerfectWindow;
	FGameplayTag State_Core_PerfectSuccess;

	// Weapon State
	FGameplayTag State_Weapon_Equipped;
	FGameplayTag State_Weapon_Equipping;
	FGameplayTag State_Weapon_Unequipping;

private:
	static FRiftGameplayTags GameplayTags;
};
