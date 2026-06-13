// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/RiftGameplayTags.h"

namespace RiftGameplayTags
{
	UE_DEFINE_GAMEPLAY_TAG(State_Attacking, "State.Attacking");
	UE_DEFINE_GAMEPLAY_TAG(State_Dodging, "State.Dodging");
	UE_DEFINE_GAMEPLAY_TAG(State_HitReact, "State.HitReact");

	UE_DEFINE_GAMEPLAY_TAG(Ability_Attack_Primary, "Ability.Attack.Primary");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Attack_Secondary, "Ability.Attack.Secondary");

	UE_DEFINE_GAMEPLAY_TAG(InputTag_Attack_Primary, "InputTag.Attack.Primary");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Attack_Secondary, "InputTag.Attack.Secondary");

	UE_DEFINE_GAMEPLAY_TAG(SetByCaller_Damage, "SetByCaller.Damage");

	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Combat_MeleeHit, "GameplayCue.Combat.MeleeHit");
}
