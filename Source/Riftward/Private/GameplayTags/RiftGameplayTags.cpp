// Fill out your copyright notice in the Description page of Project Settings.


#include "GameplayTags/RiftGameplayTags.h"

UE_DEFINE_GAMEPLAY_TAG(TAG_Data_Damage, "Data.Damage");

UE_DEFINE_GAMEPLAY_TAG(TAG_Event_Ability_TwinSword_Core_DodgeFinished,
                       "Event.Ability.TwinSword.Core.DodgeFinished");
UE_DEFINE_GAMEPLAY_TAG(TAG_Event_Ability_TwinSword_Primary_ComboWindow,
                       "Event.Ability.TwinSword.Primary.ComboWindow");
UE_DEFINE_GAMEPLAY_TAG(TAG_Event_Ability_TwinSword_Secondary_ComboWindow,
                       "Event.Ability.TwinSword.Secondary.ComboWindow");
UE_DEFINE_GAMEPLAY_TAG(TAG_Event_Ability_TwinSword_Combo_WeaponTraceBegin,
                       "Event.Ability.TwinSword.Combo.WeaponTrace.Begin");
UE_DEFINE_GAMEPLAY_TAG(TAG_Event_Ability_TwinSword_Combo_WeaponTraceTick,
                       "Event.Ability.TwinSword.Combo.WeaponTrace.Tick");
UE_DEFINE_GAMEPLAY_TAG(TAG_Event_Ability_TwinSword_Combo_WeaponTraceEnd,
                       "Event.Ability.TwinSword.Combo.WeaponTrace.End");

UE_DEFINE_GAMEPLAY_TAG(TAG_State_Ability_TwinSword_Core_PerfectDodgeWindow,
                       "State.Ability.TwinSword.Core.PerfectDodgeWindow");
UE_DEFINE_GAMEPLAY_TAG(TAG_State_Ability_TwinSword_Core_PerfectDodgeEmpowered,
                       "State.Ability.TwinSword.Core.PerfectDodgeEmpowered");
UE_DEFINE_GAMEPLAY_TAG(TAG_State_Ability_TwinSword_Core_DodgeActive,
                       "State.Ability.TwinSword.Core.DodgeActive");
UE_DEFINE_GAMEPLAY_TAG(TAG_State_Ability_TwinSword_Primary_ComboActive,
                       "State.Ability.TwinSword.Primary.ComboActive");
UE_DEFINE_GAMEPLAY_TAG(TAG_State_Ability_TwinSword_Secondary_ComboActive,
                       "State.Ability.TwinSword.Secondary.ComboActive");

FRiftGameplayTags::FRiftGameplayTags()
{
	Data_Damage = TAG_Data_Damage;

	Event_Ability_TwinSword_Core_DodgeFinished = TAG_Event_Ability_TwinSword_Core_DodgeFinished;
	Event_Ability_TwinSword_Primary_ComboWindow = TAG_Event_Ability_TwinSword_Primary_ComboWindow;
	Event_Ability_TwinSword_Secondary_ComboWindow = TAG_Event_Ability_TwinSword_Secondary_ComboWindow;
	Event_Ability_TwinSword_Combo_WeaponTraceBegin = TAG_Event_Ability_TwinSword_Combo_WeaponTraceBegin;
	Event_Ability_TwinSword_Combo_WeaponTraceTick = TAG_Event_Ability_TwinSword_Combo_WeaponTraceTick;
	Event_Ability_TwinSword_Combo_WeaponTraceEnd = TAG_Event_Ability_TwinSword_Combo_WeaponTraceEnd;

	State_Ability_TwinSword_Core_PerfectDodgeWindow = TAG_State_Ability_TwinSword_Core_PerfectDodgeWindow;
	State_Ability_TwinSword_Core_PerfectDodgeEmpowered = TAG_State_Ability_TwinSword_Core_PerfectDodgeEmpowered;
	State_Ability_TwinSword_Core_DodgeActive = TAG_State_Ability_TwinSword_Core_DodgeActive;
	State_Ability_TwinSword_Primary_ComboActive = TAG_State_Ability_TwinSword_Primary_ComboActive;
	State_Ability_TwinSword_Secondary_ComboActive = TAG_State_Ability_TwinSword_Secondary_ComboActive;
}

const FRiftGameplayTags& FRiftGameplayTags::Get()
{
	static const FRiftGameplayTags GameplayTags;
	return GameplayTags;
}
