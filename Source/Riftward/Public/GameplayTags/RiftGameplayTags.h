// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "NativeGameplayTags.h"

// Data tags
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Tag_Data_Damage);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Tag_Data_StaminaCost);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Tag_Data_ManaCost);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Tag_Data_Duration);

// Ability identity tags — named by what the ability does, not by input slot
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Tag_Ability_TwinSword_Passive_CombatFocus);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Tag_Ability_TwinSword_PrimaryCombo);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Tag_Ability_TwinSword_PrimaryHeavy);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Tag_Ability_TwinSword_SecondaryCombo);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Tag_Ability_TwinSword_SecondaryHeavy);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Tag_Ability_TwinSword_Dodge);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Tag_Ability_TwinSword_CrossWave);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Tag_Ability_TwinSword_ArmageddonBlade);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Tag_Ability_TwinSword_Ultimate);

// Event tags — scoped under ability name, no redundant "Ability" segment
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Tag_Event_TwinSword_Dodge_Finished);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Tag_Event_TwinSword_Dodge_PerfectSuccess);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Tag_Event_TwinSword_Combo_ChainPoint);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Tag_Event_TwinSword_Combo_Input_Primary);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Tag_Event_TwinSword_Combo_Input_Secondary);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Tag_Event_TwinSword_Combo_WeaponTrace_Begin);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Tag_Event_TwinSword_Combo_WeaponTrace_Tick);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Tag_Event_TwinSword_Combo_WeaponTrace_End);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Tag_Event_TwinSword_CrossWave_Execute);         // 冲击波 Montage 触发 ForwardHit 时发送
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Tag_Event_TwinSword_ArmageddonBlade_Apply);     // ArmageddonBlade Montage 触发 ApplyTimedState 时发送

// State tags — scoped under ability name, no redundant "Ability" segment
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Tag_State_TwinSword_Dodge_Active);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Tag_State_TwinSword_Dodge_Empowered);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Tag_State_TwinSword_Dodge_Heal);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Tag_State_TwinSword_PrimaryCombo_Active);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Tag_State_TwinSword_PrimaryHeavy_Active);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Tag_State_TwinSword_SecondaryCombo_Active);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Tag_State_TwinSword_SecondaryHeavy_Active);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Tag_State_TwinSword_CrossWave_Active);          // 冲击波施法期间阻塞状态
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Tag_State_TwinSword_ArmageddonBlade_Active);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Tag_State_TwinSword_Ultimate_Active);

struct FRiftGameplayTags
{
public:
	FRiftGameplayTags();

	static const FRiftGameplayTags& Get();

	// ======================
	// Data
	// ======================
	FGameplayTag Data_Damage;
	FGameplayTag Data_StaminaCost;
	FGameplayTag Data_ManaCost;
	FGameplayTag Data_Duration;

	// ======================
	// TwinSword — Ability
	// ======================
	FGameplayTag Ability_TwinSword_Passive_CombatFocus;
	FGameplayTag Ability_TwinSword_PrimaryCombo;
	FGameplayTag Ability_TwinSword_PrimaryHeavy;
	FGameplayTag Ability_TwinSword_SecondaryCombo;
	FGameplayTag Ability_TwinSword_SecondaryHeavy;
	FGameplayTag Ability_TwinSword_Dodge;
	FGameplayTag Ability_TwinSword_CrossWave;
	FGameplayTag Ability_TwinSword_ArmageddonBlade;
	FGameplayTag Ability_TwinSword_Ultimate;

	// ======================
	// TwinSword — Event
	// ======================
	FGameplayTag Event_TwinSword_Dodge_Finished;
	FGameplayTag Event_TwinSword_Dodge_PerfectSuccess;
	FGameplayTag Event_TwinSword_Combo_ChainPoint;
	FGameplayTag Event_TwinSword_Combo_Input_Primary;
	FGameplayTag Event_TwinSword_Combo_Input_Secondary;
	FGameplayTag Event_TwinSword_Combo_WeaponTrace_Begin;
	FGameplayTag Event_TwinSword_Combo_WeaponTrace_Tick;
	FGameplayTag Event_TwinSword_Combo_WeaponTrace_End;
	FGameplayTag Event_TwinSword_CrossWave_Execute;         // 冲击波 Montage 触发 ForwardHit 时发送
	FGameplayTag Event_TwinSword_ArmageddonBlade_Apply;     // ArmageddonBlade Montage 触发 ApplyTimedState 时发送

	// ======================
	// TwinSword — State
	// ======================
	FGameplayTag State_TwinSword_Dodge_Active;
	FGameplayTag State_TwinSword_Dodge_Empowered;
	FGameplayTag State_TwinSword_Dodge_Heal;
	FGameplayTag State_TwinSword_PrimaryCombo_Active;
	FGameplayTag State_TwinSword_PrimaryHeavy_Active;
	FGameplayTag State_TwinSword_SecondaryCombo_Active;
	FGameplayTag State_TwinSword_SecondaryHeavy_Active;
	FGameplayTag State_TwinSword_CrossWave_Active;          // 冲击波施法期间阻塞状态
	FGameplayTag State_TwinSword_ArmageddonBlade_Active;
	FGameplayTag State_TwinSword_Ultimate_Active;
};
