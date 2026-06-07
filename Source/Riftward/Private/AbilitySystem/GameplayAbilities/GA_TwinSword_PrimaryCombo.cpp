// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/GameplayAbilities/GA_TwinSword_PrimaryCombo.h"
#include "Data/Player/Input/AbilityInputID.h"
#include "GameplayTags/RiftGameplayTags.h"

UGA_TwinSword_PrimaryCombo::UGA_TwinSword_PrimaryCombo()
{
	AbilityInputID = EAbilityInputID::Primary;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

FGameplayTag UGA_TwinSword_PrimaryCombo::GetAbilityActiveStateTag() const
{
	return FRiftGameplayTags::Get().State_TwinSword_PrimaryCombo_Active;
}

bool UGA_TwinSword_PrimaryCombo::ShouldCommitComboAbility() const
{
	return false;
}
