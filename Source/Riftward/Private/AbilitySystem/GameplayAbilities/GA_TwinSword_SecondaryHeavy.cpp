// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/GameplayAbilities/GA_TwinSword_SecondaryHeavy.h"

#include "Data/Player/Input/AbilityInputID.h"
#include "GameplayTags/RiftGameplayTags.h"

UGA_TwinSword_SecondaryHeavy::UGA_TwinSword_SecondaryHeavy()
{
	AbilityInputID = EAbilityInputID::SecondaryHeavy;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

FGameplayTag UGA_TwinSword_SecondaryHeavy::GetAbilityActiveStateTag() const
{
	return FRiftGameplayTags::Get().State_TwinSword_SecondaryHeavy_Active;
}

bool UGA_TwinSword_SecondaryHeavy::ShouldCommitComboAbility() const
{
	return true;
}
