// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "NativeGameplayTags.h"

UE_DECLARE_GAMEPLAY_TAG_EXTERN(Tag_Data_Damage);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Tag_Data_StaminaCost);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Tag_Event_Ability_TwinSword_Core_DodgeFinished);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Tag_Event_Ability_TwinSword_Primary_ComboWindow);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Tag_Event_Ability_TwinSword_Secondary_ComboWindow);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Tag_Event_Ability_TwinSword_Combo_WeaponTraceBegin);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Tag_Event_Ability_TwinSword_Combo_WeaponTraceTick);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Tag_Event_Ability_TwinSword_Combo_WeaponTraceEnd);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Tag_State_Ability_TwinSword_Core_PerfectDodgeWindow);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Tag_State_Ability_TwinSword_Core_PerfectDodgeEmpowered);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Tag_State_Ability_TwinSword_Core_DodgeActive);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Tag_State_Ability_TwinSword_Primary_ComboActive);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Tag_State_Ability_TwinSword_Secondary_ComboActive);

struct FRiftGameplayTags
{
public:
	FRiftGameplayTags();

	static const FRiftGameplayTags& Get();

	// ======================
	// TwinSword
	// ======================
	/* Data */
	FGameplayTag Data_Damage;

	FGameplayTag Data_StaminaCost;

	/* Event */
	FGameplayTag Event_Ability_TwinSword_Core_DodgeFinished;

	FGameplayTag Event_Ability_TwinSword_Primary_ComboWindow;

	FGameplayTag Event_Ability_TwinSword_Secondary_ComboWindow;

	FGameplayTag Event_Ability_TwinSword_Combo_WeaponTraceBegin;

	FGameplayTag Event_Ability_TwinSword_Combo_WeaponTraceTick;

	FGameplayTag Event_Ability_TwinSword_Combo_WeaponTraceEnd;

	/* State */
	FGameplayTag State_Ability_TwinSword_Core_PerfectDodgeWindow;

	FGameplayTag State_Ability_TwinSword_Core_PerfectDodgeEmpowered;

	FGameplayTag State_Ability_TwinSword_Core_DodgeActive;

	FGameplayTag State_Ability_TwinSword_Primary_ComboActive;

	FGameplayTag State_Ability_TwinSword_Secondary_ComboActive;
};
