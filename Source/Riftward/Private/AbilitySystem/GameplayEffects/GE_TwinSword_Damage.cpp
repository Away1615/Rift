// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/GameplayEffects/GE_TwinSword_Damage.h"
#include "AbilitySystem/Attributes/HealthAttributeSet.h"
#include "GameplayTags/RiftGameplayTags.h"

UGE_TwinSword_Damage::UGE_TwinSword_Damage()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;

	FSetByCallerFloat SetByCallerDamage;
	SetByCallerDamage.DataTag = FRiftGameplayTags::Get().Data_Damage;

	FGameplayModifierInfo ModifierInfo;
	ModifierInfo.Attribute = UHealthAttributeSet::GetHealthAttribute();
	ModifierInfo.ModifierOp = EGameplayModOp::Additive;
	ModifierInfo.ModifierMagnitude = FGameplayEffectModifierMagnitude(SetByCallerDamage);

	Modifiers.Add(ModifierInfo);
}
