// Fill out your copyright notice in the Description page of Project Settings.


#include "GameplayTags/RiftGameplayTags.h"

UE_DEFINE_GAMEPLAY_TAG(Tag_Data_Damage,       "Data.Damage");
UE_DEFINE_GAMEPLAY_TAG(Tag_Data_StaminaCost,  "Data.StaminaCost");
UE_DEFINE_GAMEPLAY_TAG(Tag_Data_ManaCost,     "Data.ManaCost");
UE_DEFINE_GAMEPLAY_TAG(Tag_Data_Duration,     "Data.Duration");

// Ability identity tags
UE_DEFINE_GAMEPLAY_TAG(Tag_Ability_TwinSword_Passive_CombatFocus, "Ability.TwinSword.Passive.CombatFocus");
UE_DEFINE_GAMEPLAY_TAG(Tag_Ability_TwinSword_PrimaryCombo,        "Ability.TwinSword.PrimaryCombo");
UE_DEFINE_GAMEPLAY_TAG(Tag_Ability_TwinSword_PrimaryHeavy,        "Ability.TwinSword.PrimaryHeavy");
UE_DEFINE_GAMEPLAY_TAG(Tag_Ability_TwinSword_SecondaryCombo,      "Ability.TwinSword.SecondaryCombo");
UE_DEFINE_GAMEPLAY_TAG(Tag_Ability_TwinSword_SecondaryHeavy,      "Ability.TwinSword.SecondaryHeavy");
UE_DEFINE_GAMEPLAY_TAG(Tag_Ability_TwinSword_Dodge,               "Ability.TwinSword.Dodge");
UE_DEFINE_GAMEPLAY_TAG(Tag_Ability_TwinSword_CrossWave,           "Ability.TwinSword.CrossWave");
UE_DEFINE_GAMEPLAY_TAG(Tag_Ability_TwinSword_ArmageddonBlade,     "Ability.TwinSword.ArmageddonBlade");
UE_DEFINE_GAMEPLAY_TAG(Tag_Ability_TwinSword_Ultimate,            "Ability.TwinSword.Ultimate");

// Event tags
UE_DEFINE_GAMEPLAY_TAG(Tag_Event_TwinSword_Dodge_Finished,          "Event.TwinSword.Dodge.Finished");
UE_DEFINE_GAMEPLAY_TAG(Tag_Event_TwinSword_Dodge_PerfectSuccess,    "Event.TwinSword.Dodge.PerfectSuccess");
UE_DEFINE_GAMEPLAY_TAG(Tag_Event_TwinSword_Combo_ChainPoint,        "Event.TwinSword.Combo.ChainPoint");
UE_DEFINE_GAMEPLAY_TAG(Tag_Event_TwinSword_Combo_Input_Primary,      "Event.TwinSword.Combo.Input.Primary");
UE_DEFINE_GAMEPLAY_TAG(Tag_Event_TwinSword_Combo_Input_Secondary,    "Event.TwinSword.Combo.Input.Secondary");
UE_DEFINE_GAMEPLAY_TAG(Tag_Event_TwinSword_Combo_WeaponTrace_Begin,     "Event.TwinSword.Combo.WeaponTrace.Begin");
UE_DEFINE_GAMEPLAY_TAG(Tag_Event_TwinSword_Combo_WeaponTrace_Tick,      "Event.TwinSword.Combo.WeaponTrace.Tick");
UE_DEFINE_GAMEPLAY_TAG(Tag_Event_TwinSword_Combo_WeaponTrace_End,       "Event.TwinSword.Combo.WeaponTrace.End");
UE_DEFINE_GAMEPLAY_TAG(Tag_Event_TwinSword_CrossWave_Execute,           "Event.TwinSword.CrossWave.Execute");
UE_DEFINE_GAMEPLAY_TAG(Tag_Event_TwinSword_ArmageddonBlade_Apply,       "Event.TwinSword.ArmageddonBlade.Apply");

// State tags
UE_DEFINE_GAMEPLAY_TAG(Tag_State_TwinSword_Dodge_Active,                "State.TwinSword.Dodge.Active");
UE_DEFINE_GAMEPLAY_TAG(Tag_State_TwinSword_Dodge_Empowered,             "State.TwinSword.Dodge.Empowered");
UE_DEFINE_GAMEPLAY_TAG(Tag_State_TwinSword_Dodge_Heal,                  "State.TwinSword.Dodge.Heal");
UE_DEFINE_GAMEPLAY_TAG(Tag_State_TwinSword_PrimaryCombo_Active,         "State.TwinSword.PrimaryCombo.Active");
UE_DEFINE_GAMEPLAY_TAG(Tag_State_TwinSword_PrimaryHeavy_Active,         "State.TwinSword.PrimaryHeavy.Active");
UE_DEFINE_GAMEPLAY_TAG(Tag_State_TwinSword_SecondaryCombo_Active,       "State.TwinSword.SecondaryCombo.Active");
UE_DEFINE_GAMEPLAY_TAG(Tag_State_TwinSword_SecondaryHeavy_Active,      "State.TwinSword.SecondaryHeavy.Active");
UE_DEFINE_GAMEPLAY_TAG(Tag_State_TwinSword_CrossWave_Active,            "State.TwinSword.CrossWave.Active");
UE_DEFINE_GAMEPLAY_TAG(Tag_State_TwinSword_ArmageddonBlade_Active,      "State.TwinSword.ArmageddonBlade.Active");
UE_DEFINE_GAMEPLAY_TAG(Tag_State_TwinSword_Ultimate_Active,             "State.TwinSword.Ultimate.Active");

FRiftGameplayTags::FRiftGameplayTags()
{
	Data_Damage      = Tag_Data_Damage;
	Data_StaminaCost = Tag_Data_StaminaCost;
	Data_ManaCost    = Tag_Data_ManaCost;
	Data_Duration    = Tag_Data_Duration;

	Ability_TwinSword_Passive_CombatFocus = Tag_Ability_TwinSword_Passive_CombatFocus;
	Ability_TwinSword_PrimaryCombo        = Tag_Ability_TwinSword_PrimaryCombo;
	Ability_TwinSword_PrimaryHeavy        = Tag_Ability_TwinSword_PrimaryHeavy;
	Ability_TwinSword_SecondaryCombo      = Tag_Ability_TwinSword_SecondaryCombo;
	Ability_TwinSword_SecondaryHeavy      = Tag_Ability_TwinSword_SecondaryHeavy;
	Ability_TwinSword_Dodge               = Tag_Ability_TwinSword_Dodge;
	Ability_TwinSword_CrossWave           = Tag_Ability_TwinSword_CrossWave;
	Ability_TwinSword_ArmageddonBlade     = Tag_Ability_TwinSword_ArmageddonBlade;
	Ability_TwinSword_Ultimate            = Tag_Ability_TwinSword_Ultimate;

	Event_TwinSword_Dodge_Finished          = Tag_Event_TwinSword_Dodge_Finished;
	Event_TwinSword_Dodge_PerfectSuccess    = Tag_Event_TwinSword_Dodge_PerfectSuccess;
	Event_TwinSword_Combo_ChainPoint        = Tag_Event_TwinSword_Combo_ChainPoint;
	Event_TwinSword_Combo_Input_Primary     = Tag_Event_TwinSword_Combo_Input_Primary;
	Event_TwinSword_Combo_Input_Secondary   = Tag_Event_TwinSword_Combo_Input_Secondary;
	Event_TwinSword_Combo_WeaponTrace_Begin   = Tag_Event_TwinSword_Combo_WeaponTrace_Begin;
	Event_TwinSword_Combo_WeaponTrace_Tick    = Tag_Event_TwinSword_Combo_WeaponTrace_Tick;
	Event_TwinSword_Combo_WeaponTrace_End     = Tag_Event_TwinSword_Combo_WeaponTrace_End;
	Event_TwinSword_CrossWave_Execute         = Tag_Event_TwinSword_CrossWave_Execute;
	Event_TwinSword_ArmageddonBlade_Apply     = Tag_Event_TwinSword_ArmageddonBlade_Apply;

	State_TwinSword_Dodge_Active              = Tag_State_TwinSword_Dodge_Active;
	State_TwinSword_Dodge_Empowered           = Tag_State_TwinSword_Dodge_Empowered;
	State_TwinSword_Dodge_Heal                = Tag_State_TwinSword_Dodge_Heal;
	State_TwinSword_PrimaryCombo_Active       = Tag_State_TwinSword_PrimaryCombo_Active;
	State_TwinSword_PrimaryHeavy_Active       = Tag_State_TwinSword_PrimaryHeavy_Active;
	State_TwinSword_SecondaryCombo_Active     = Tag_State_TwinSword_SecondaryCombo_Active;
	State_TwinSword_SecondaryHeavy_Active     = Tag_State_TwinSword_SecondaryHeavy_Active;
	State_TwinSword_CrossWave_Active          = Tag_State_TwinSword_CrossWave_Active;
	State_TwinSword_ArmageddonBlade_Active    = Tag_State_TwinSword_ArmageddonBlade_Active;
	State_TwinSword_Ultimate_Active           = Tag_State_TwinSword_Ultimate_Active;
}

const FRiftGameplayTags& FRiftGameplayTags::Get()
{
	static const FRiftGameplayTags GameplayTags;
	return GameplayTags;
}
