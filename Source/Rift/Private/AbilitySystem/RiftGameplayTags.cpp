// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/RiftGameplayTags.h"

namespace RiftGameplayTags
{
	UE_DEFINE_GAMEPLAY_TAG(State_Attacking, "State.Attacking");
	UE_DEFINE_GAMEPLAY_TAG(State_Dodging, "State.Dodging");
	UE_DEFINE_GAMEPLAY_TAG(State_Action_Cancelable, "State.Action.Cancelable");
	UE_DEFINE_GAMEPLAY_TAG(State_Staggered, "State.Staggered");
	UE_DEFINE_GAMEPLAY_TAG(State_Invincible, "State.Invincible");
	UE_DEFINE_GAMEPLAY_TAG(State_Blocking, "State.Blocking");
	UE_DEFINE_GAMEPLAY_TAG(State_Attack_RapidSlash, "State.Attack.RapidSlash");
	UE_DEFINE_GAMEPLAY_TAG(State_Attack_RapidSlashReady, "State.Attack.RapidSlashReady");
	UE_DEFINE_GAMEPLAY_TAG(State_Intro, "State.Intro");
	UE_DEFINE_GAMEPLAY_TAG(State_Enemy_Intro, "State.Enemy.Intro");
	UE_DEFINE_GAMEPLAY_TAG(State_Enemy_SuperArmor, "State.Enemy.SuperArmor");
	UE_DEFINE_GAMEPLAY_TAG(State_SuperArmor_Red, "State.SuperArmor.Red");
	UE_DEFINE_GAMEPLAY_TAG(State_Hit_Light, "State.Hit.Light");
	UE_DEFINE_GAMEPLAY_TAG(State_Hit_Heavy, "State.Hit.Heavy");
	UE_DEFINE_GAMEPLAY_TAG(State_Dead, "State.Dead");

	UE_DEFINE_GAMEPLAY_TAG(Ability_Attack_Primary, "Ability.Attack.Primary");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Attack_Secondary, "Ability.Attack.Secondary");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Attack_Combo, "Ability.Attack.Combo");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Attack_TwinSwordCombo, "Ability.Attack.TwinSwordCombo");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Attack_TwinSwordRapidSlash, "Ability.Attack.TwinSwordRapidSlash");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Skill_TwinSword_Q, "Ability.Skill.TwinSword.Q");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Skill_TwinSword_SwordWave, "Ability.Skill.TwinSword.SwordWave");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Enemy_MeleeAttack, "Ability.Enemy.MeleeAttack");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Enemy_ShieldBlock, "Ability.Enemy.ShieldBlock");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Dodge, "Ability.Dodge");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Guard, "Ability.Guard");

	UE_DEFINE_GAMEPLAY_TAG(InputTag_Attack_Primary, "InputTag.Attack.Primary");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Attack_Secondary, "InputTag.Attack.Secondary");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Dodge, "InputTag.Dodge");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Guard, "InputTag.Guard");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Skill_Q, "InputTag.Skill.Q");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Skill_E, "InputTag.Skill.E");

	UE_DEFINE_GAMEPLAY_TAG(SetByCaller_Damage, "SetByCaller.Damage");
	UE_DEFINE_GAMEPLAY_TAG(SetByCaller_PoiseDamage, "SetByCaller.PoiseDamage");
	UE_DEFINE_GAMEPLAY_TAG(SetByCaller_UltimateCharge, "SetByCaller.UltimateCharge");

	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Combat_MeleeHit, "GameplayCue.Combat.MeleeHit");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Combat_BlockedHit, "GameplayCue.Combat.BlockedHit");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Combat_PlayerHit, "GameplayCue.Combat.PlayerHit");
}
