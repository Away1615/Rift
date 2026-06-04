// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/GameplayAbilities/GA_TwinSword_Secondary.h"
#include "Data/Player/Input/AbilityInputID.h"
#include "GameplayTags/RiftGameplayTags.h"

UGA_TwinSword_Secondary::UGA_TwinSword_Secondary()
{
	AbilityInputID = EAbilityInputID::Secondary;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

FGameplayTag UGA_TwinSword_Secondary::GetComboWindowEventTag() const
{
	return FRiftGameplayTags::Get().Event_Ability_TwinSword_Secondary_ComboWindow;
}

FGameplayTag UGA_TwinSword_Secondary::GetAbilityActiveStateTag() const
{
	return FRiftGameplayTags::Get().State_Ability_TwinSword_Secondary_ComboActive;
}

bool UGA_TwinSword_Secondary::ShouldCommitComboAbility() const
{
	return true;
}
