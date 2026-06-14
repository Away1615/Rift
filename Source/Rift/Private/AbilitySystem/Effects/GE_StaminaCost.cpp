#include "AbilitySystem/Effects/GE_StaminaCost.h"

#include "AbilitySystem/Attributes/RiftPlayerAttributeSet.h"
#include "AbilitySystem/RiftGameplayTags.h"

UGE_StaminaCost::UGE_StaminaCost()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;

	FSetByCallerFloat StaminaCostMagnitude;
	StaminaCostMagnitude.DataTag = RiftGameplayTags::SetByCaller_StaminaCost;

	FGameplayModifierInfo StaminaCostModifier;
	StaminaCostModifier.Attribute = URiftPlayerAttributeSet::GetStaminaAttribute();
	StaminaCostModifier.ModifierOp = EGameplayModOp::Additive;
	StaminaCostModifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(StaminaCostMagnitude);

	Modifiers.Add(StaminaCostModifier);
}
