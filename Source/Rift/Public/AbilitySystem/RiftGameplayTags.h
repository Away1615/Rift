// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "NativeGameplayTags.h"

namespace RiftGameplayTags
{
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Attacking);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Dodging);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Staggered);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Invincible);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_SuperArmor_Red);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Hit_Heavy);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Dead);

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Attack_Primary);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Attack_Secondary);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Attack_Combo);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Attack_Heavy);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Enemy_MeleeAttack);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Dodge);

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Attack_Primary);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Attack_Secondary);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Attack_PrimaryHeavy);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Attack_SecondaryHeavy);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Core);

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(SetByCaller_Damage);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(SetByCaller_PoiseDamage);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(SetByCaller_StaminaRegen);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(SetByCaller_StaminaCost);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(SetByCaller_UltimateCharge);

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Combat_MeleeHit);
}
