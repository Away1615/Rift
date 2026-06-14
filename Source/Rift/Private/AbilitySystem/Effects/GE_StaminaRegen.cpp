// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Effects/GE_StaminaRegen.h"

#include "AbilitySystem/Attributes/RiftPlayerAttributeSet.h"
#include "AbilitySystem/RiftGameplayTags.h"

UGE_StaminaRegen::UGE_StaminaRegen()
{
	DurationPolicy = EGameplayEffectDurationType::Infinite;
	Period = 0.1f;

	FSetByCallerFloat StaminaRegenMagnitude;
	StaminaRegenMagnitude.DataTag = RiftGameplayTags::SetByCaller_StaminaRegen;

	FGameplayModifierInfo StaminaRegenModifier;
	StaminaRegenModifier.Attribute = URiftPlayerAttributeSet::GetStaminaAttribute();
	StaminaRegenModifier.ModifierOp = EGameplayModOp::Additive;
	StaminaRegenModifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(StaminaRegenMagnitude);

	Modifiers.Add(StaminaRegenModifier);
}
