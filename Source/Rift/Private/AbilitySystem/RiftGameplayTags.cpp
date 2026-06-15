// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/RiftGameplayTags.h"

namespace RiftGameplayTags
{
	UE_DEFINE_GAMEPLAY_TAG(State_Attacking, "State.Attacking");
	UE_DEFINE_GAMEPLAY_TAG(State_Dodging, "State.Dodging");
	UE_DEFINE_GAMEPLAY_TAG(State_HitReact, "State.HitReact");
	UE_DEFINE_GAMEPLAY_TAG(State_Invincible, "State.Invincible");
	UE_DEFINE_GAMEPLAY_TAG(State_Dead, "State.Dead");

	UE_DEFINE_GAMEPLAY_TAG(Ability_Attack_Primary, "Ability.Attack.Primary");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Attack_Secondary, "Ability.Attack.Secondary");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Attack_Combo, "Ability.Attack.Combo");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Attack_Heavy, "Ability.Attack.Heavy");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Enemy_MeleeAttack, "Ability.Enemy.MeleeAttack");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Dodge, "Ability.Dodge");

	UE_DEFINE_GAMEPLAY_TAG(InputTag_Attack_Primary, "InputTag.Attack.Primary");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Attack_Secondary, "InputTag.Attack.Secondary");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Attack_PrimaryHeavy, "InputTag.Attack.PrimaryHeavy");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Attack_SecondaryHeavy, "InputTag.Attack.SecondaryHeavy");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Core, "InputTag.Core");

	UE_DEFINE_GAMEPLAY_TAG(SetByCaller_Damage, "SetByCaller.Damage");
	UE_DEFINE_GAMEPLAY_TAG(SetByCaller_PoiseDamage, "SetByCaller.PoiseDamage");
	UE_DEFINE_GAMEPLAY_TAG(SetByCaller_StaminaRegen, "SetByCaller.StaminaRegen");
	UE_DEFINE_GAMEPLAY_TAG(SetByCaller_StaminaCost, "SetByCaller.StaminaCost");
	UE_DEFINE_GAMEPLAY_TAG(SetByCaller_SwordIntent, "SetByCaller.SwordIntent");
	UE_DEFINE_GAMEPLAY_TAG(SetByCaller_UltimateCharge, "SetByCaller.UltimateCharge");

	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Combat_MeleeHit, "GameplayCue.Combat.MeleeHit");
}
