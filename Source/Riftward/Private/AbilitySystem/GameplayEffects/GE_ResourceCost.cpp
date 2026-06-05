#include "AbilitySystem/GameplayEffects/GE_ResourceCost.h"

#include "AbilitySystem/Attributes/ResourceAttributeSet.h"
#include "GameplayTags/RiftGameplayTags.h"

UGE_ResourceCost::UGE_ResourceCost()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;

	FSetByCallerFloat SetByCallerStaminaCost;
	SetByCallerStaminaCost.DataTag = FRiftGameplayTags::Get().Data_StaminaCost;

	FGameplayModifierInfo StaminaModifier;
	StaminaModifier.Attribute = UResourceAttributeSet::GetStaminaAttribute();
	StaminaModifier.ModifierOp = EGameplayModOp::Additive;
	StaminaModifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(SetByCallerStaminaCost);
	Modifiers.Add(StaminaModifier);

	FSetByCallerFloat SetByCallerManaCost;
	SetByCallerManaCost.DataTag = FRiftGameplayTags::Get().Data_ManaCost;

	FGameplayModifierInfo ManaModifier;
	ManaModifier.Attribute = UResourceAttributeSet::GetManaAttribute();
	ManaModifier.ModifierOp = EGameplayModOp::Additive;
	ManaModifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(SetByCallerManaCost);
	Modifiers.Add(ManaModifier);
}
