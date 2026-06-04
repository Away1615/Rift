// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/GameplayAbilities/GA_TwinSword_Primary.h"
#include "Data/Player/Input/AbilityInputID.h"
#include "GameplayTags/RiftGameplayTags.h"

UGA_TwinSword_Primary::UGA_TwinSword_Primary()
{
	AbilityInputID = EAbilityInputID::Primary;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

FGameplayTag UGA_TwinSword_Primary::GetComboWindowEventTag() const
{
	return FRiftGameplayTags::Get().Event_Ability_TwinSword_Primary_ComboWindow;
}

FGameplayTag UGA_TwinSword_Primary::GetAbilityActiveStateTag() const
{
	return FRiftGameplayTags::Get().State_Ability_TwinSword_Primary_ComboActive;
}

bool UGA_TwinSword_Primary::ShouldCommitComboAbility() const
{
	return false;
}
