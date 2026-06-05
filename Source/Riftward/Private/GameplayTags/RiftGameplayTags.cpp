// Fill out your copyright notice in the Description page of Project Settings.


#include "GameplayTags/RiftGameplayTags.h"

UE_DEFINE_GAMEPLAY_TAG(Tag_Data_Damage, "Data.Damage");
UE_DEFINE_GAMEPLAY_TAG(Tag_Data_StaminaCost, "Data.StaminaCost");
UE_DEFINE_GAMEPLAY_TAG(Tag_Data_ManaCost, "Data.ManaCost");

UE_DEFINE_GAMEPLAY_TAG(Tag_Ability_TwinSword_Core,
                       "Ability.TwinSword.Core");
UE_DEFINE_GAMEPLAY_TAG(Tag_Ability_TwinSword_Primary,
                       "Ability.TwinSword.Primary");
UE_DEFINE_GAMEPLAY_TAG(Tag_Ability_TwinSword_Secondary,
                       "Ability.TwinSword.Secondary");
UE_DEFINE_GAMEPLAY_TAG(Tag_Ability_TwinSword_Signature,
                       "Ability.TwinSword.Signature");
UE_DEFINE_GAMEPLAY_TAG(Tag_Ability_TwinSword_Enhance,
                       "Ability.TwinSword.Enhance");

UE_DEFINE_GAMEPLAY_TAG(Tag_Event_Ability_TwinSword_Core_DodgeFinished,
                       "Event.Ability.TwinSword.Core.DodgeFinished");
UE_DEFINE_GAMEPLAY_TAG(Tag_Event_Ability_TwinSword_Core_PerfectDodgeSuccess,
                       "Event.Ability.TwinSword.Core.PerfectDodgeSuccess");
UE_DEFINE_GAMEPLAY_TAG(Tag_Event_Ability_TwinSword_Primary_ComboWindow,
                       "Event.Ability.TwinSword.Primary.ComboWindow");
UE_DEFINE_GAMEPLAY_TAG(Tag_Event_Ability_TwinSword_Secondary_ComboWindow,
                       "Event.Ability.TwinSword.Secondary.ComboWindow");
UE_DEFINE_GAMEPLAY_TAG(Tag_Event_Ability_TwinSword_Combo_WeaponTraceBegin,
                       "Event.Ability.TwinSword.Combo.WeaponTrace.Begin");
UE_DEFINE_GAMEPLAY_TAG(Tag_Event_Ability_TwinSword_Combo_WeaponTraceTick,
                       "Event.Ability.TwinSword.Combo.WeaponTrace.Tick");
UE_DEFINE_GAMEPLAY_TAG(Tag_Event_Ability_TwinSword_Combo_WeaponTraceEnd,
                       "Event.Ability.TwinSword.Combo.WeaponTrace.End");

UE_DEFINE_GAMEPLAY_TAG(Tag_State_Ability_TwinSword_Core_PerfectDodgeWindow,
                       "State.Ability.TwinSword.Core.PerfectDodgeWindow");
UE_DEFINE_GAMEPLAY_TAG(Tag_State_Ability_TwinSword_Core_PerfectDodgeEmpowered,
                       "State.Ability.TwinSword.Core.PerfectDodgeEmpowered");
UE_DEFINE_GAMEPLAY_TAG(Tag_State_Ability_TwinSword_Core_DodgeActive,
                       "State.Ability.TwinSword.Core.DodgeActive");
UE_DEFINE_GAMEPLAY_TAG(Tag_State_Ability_TwinSword_Primary_ComboActive,
                       "State.Ability.TwinSword.Primary.ComboActive");
UE_DEFINE_GAMEPLAY_TAG(Tag_State_Ability_TwinSword_Secondary_ComboActive,
                       "State.Ability.TwinSword.Secondary.ComboActive");
UE_DEFINE_GAMEPLAY_TAG(Tag_State_Ability_TwinSword_Enhance_Active,
                       "State.Ability.TwinSword.Enhance.Active");

FRiftGameplayTags::FRiftGameplayTags()
{
	Data_Damage = Tag_Data_Damage;
	Data_StaminaCost = Tag_Data_StaminaCost;
	Data_ManaCost = Tag_Data_ManaCost;

	Ability_TwinSword_Core = Tag_Ability_TwinSword_Core;
	Ability_TwinSword_Primary = Tag_Ability_TwinSword_Primary;
	Ability_TwinSword_Secondary = Tag_Ability_TwinSword_Secondary;
	Ability_TwinSword_Signature = Tag_Ability_TwinSword_Signature;
	Ability_TwinSword_Enhance = Tag_Ability_TwinSword_Enhance;

	Event_Ability_TwinSword_Core_DodgeFinished = Tag_Event_Ability_TwinSword_Core_DodgeFinished;
	Event_Ability_TwinSword_Core_PerfectDodgeSuccess = Tag_Event_Ability_TwinSword_Core_PerfectDodgeSuccess;
	Event_Ability_TwinSword_Primary_ComboWindow = Tag_Event_Ability_TwinSword_Primary_ComboWindow;
	Event_Ability_TwinSword_Secondary_ComboWindow = Tag_Event_Ability_TwinSword_Secondary_ComboWindow;
	Event_Ability_TwinSword_Combo_WeaponTraceBegin = Tag_Event_Ability_TwinSword_Combo_WeaponTraceBegin;
	Event_Ability_TwinSword_Combo_WeaponTraceTick = Tag_Event_Ability_TwinSword_Combo_WeaponTraceTick;
	Event_Ability_TwinSword_Combo_WeaponTraceEnd = Tag_Event_Ability_TwinSword_Combo_WeaponTraceEnd;

	State_Ability_TwinSword_Core_PerfectDodgeWindow = Tag_State_Ability_TwinSword_Core_PerfectDodgeWindow;
	State_Ability_TwinSword_Core_PerfectDodgeEmpowered = Tag_State_Ability_TwinSword_Core_PerfectDodgeEmpowered;
	State_Ability_TwinSword_Core_DodgeActive = Tag_State_Ability_TwinSword_Core_DodgeActive;
	State_Ability_TwinSword_Primary_ComboActive = Tag_State_Ability_TwinSword_Primary_ComboActive;
	State_Ability_TwinSword_Secondary_ComboActive = Tag_State_Ability_TwinSword_Secondary_ComboActive;
	State_Ability_TwinSword_Enhance_Active = Tag_State_Ability_TwinSword_Enhance_Active;
}

const FRiftGameplayTags& FRiftGameplayTags::Get()
{
	static const FRiftGameplayTags GameplayTags;
	return GameplayTags;
}
