// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/GameplayEffects/GE_StaminaCost.h"

#include "AbilitySystem/Attributes/ResourceAttributeSet.h"
#include "GameplayTags/RiftGameplayTags.h"

UGE_StaminaCost::UGE_StaminaCost()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;

	FSetByCallerFloat SetByCallerCost;
	SetByCallerCost.DataTag = FRiftGameplayTags::Get().Data_StaminaCost;

	FGameplayModifierInfo ModifierInfo;
	ModifierInfo.Attribute = UResourceAttributeSet::GetStaminaAttribute();
	ModifierInfo.ModifierOp = EGameplayModOp::Additive;
	ModifierInfo.ModifierMagnitude = FGameplayEffectModifierMagnitude(SetByCallerCost);

	Modifiers.Add(ModifierInfo);
}
