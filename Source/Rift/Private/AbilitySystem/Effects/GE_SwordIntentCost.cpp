#include "AbilitySystem/Effects/GE_SwordIntentCost.h"

#include "AbilitySystem/Attributes/RiftResourceAttributeSet.h"
#include "AbilitySystem/RiftGameplayTags.h"

UGE_SwordIntentCost::UGE_SwordIntentCost()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;

	FSetByCallerFloat SwordIntentCostMagnitude;
	SwordIntentCostMagnitude.DataTag = RiftGameplayTags::SetByCaller_SwordIntent;

	FGameplayModifierInfo SwordIntentCostModifier;
	SwordIntentCostModifier.Attribute = URiftResourceAttributeSet::GetSwordIntentAttribute();
	SwordIntentCostModifier.ModifierOp = EGameplayModOp::Additive;
	SwordIntentCostModifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(SwordIntentCostMagnitude);

	Modifiers.Add(SwordIntentCostModifier);
}
