// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Effects/GE_MeleeDamage.h"

#include "AbilitySystem/Attributes/RiftEnemyAttributeSet.h"
#include "AbilitySystem/RiftGameplayTags.h"
#include "GameplayEffectComponents/TargetTagRequirementsGameplayEffectComponent.h"

UGE_MeleeDamage::UGE_MeleeDamage()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;

	UTargetTagRequirementsGameplayEffectComponent* TargetTagRequirementsComponent =
		CreateDefaultSubobject<UTargetTagRequirementsGameplayEffectComponent>(TEXT("TargetTagRequirements"));
	TargetTagRequirementsComponent->ApplicationTagRequirements.IgnoreTags.AddTag(RiftGameplayTags::State_Dodging);
	GEComponents.Add(TargetTagRequirementsComponent);

	FSetByCallerFloat DamageMagnitude;
	DamageMagnitude.DataTag = RiftGameplayTags::SetByCaller_Damage;

	FGameplayModifierInfo DamageModifier;
	DamageModifier.Attribute = URiftEnemyAttributeSet::GetDamageAttribute();
	DamageModifier.ModifierOp = EGameplayModOp::Additive;
	DamageModifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(DamageMagnitude);

	Modifiers.Add(DamageModifier);

	FSetByCallerFloat PoiseDamageMagnitude;
	PoiseDamageMagnitude.DataTag = RiftGameplayTags::SetByCaller_PoiseDamage;

	FGameplayModifierInfo PoiseDamageModifier;
	PoiseDamageModifier.Attribute = URiftEnemyAttributeSet::GetPoiseDamageAttribute();
	PoiseDamageModifier.ModifierOp = EGameplayModOp::Additive;
	PoiseDamageModifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(PoiseDamageMagnitude);

	Modifiers.Add(PoiseDamageModifier);
}
