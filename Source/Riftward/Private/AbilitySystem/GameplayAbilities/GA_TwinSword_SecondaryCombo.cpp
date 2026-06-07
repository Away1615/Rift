// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/GameplayAbilities/GA_TwinSword_SecondaryCombo.h"
#include "Data/Player/Input/AbilityInputID.h"
#include "GameplayTags/RiftGameplayTags.h"

UGA_TwinSword_SecondaryCombo::UGA_TwinSword_SecondaryCombo()
{
	AbilityInputID = EAbilityInputID::Secondary;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

FGameplayTag UGA_TwinSword_SecondaryCombo::GetAbilityActiveStateTag() const
{
	return FRiftGameplayTags::Get().State_TwinSword_SecondaryCombo_Active;
}

bool UGA_TwinSword_SecondaryCombo::ShouldCommitComboAbility() const
{
	return true;
}
