// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Effects/GE_MeleeDamage.h"

#include "AbilitySystem/Attributes/RiftEnemyAttributeSet.h"
#include "AbilitySystem/RiftGameplayTags.h"

UGE_MeleeDamage::UGE_MeleeDamage()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;

	FSetByCallerFloat DamageMagnitude;
	DamageMagnitude.DataTag = RiftGameplayTags::SetByCaller_Damage;

	FGameplayModifierInfo DamageModifier;
	DamageModifier.Attribute = URiftEnemyAttributeSet::GetDamageAttribute();
	DamageModifier.ModifierOp = EGameplayModOp::Additive;
	DamageModifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(DamageMagnitude);

	Modifiers.Add(DamageModifier);
}
