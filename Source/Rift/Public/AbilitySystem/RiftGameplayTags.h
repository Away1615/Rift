// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "NativeGameplayTags.h"

namespace RiftGameplayTags
{
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Attacking);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Dodging);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_HitReact);

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Attack_Primary);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Attack_Secondary);

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Attack_Primary);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Attack_Secondary);
}
