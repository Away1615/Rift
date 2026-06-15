#include "AbilitySystem/Effects/GE_GainResource.h"

#include "AbilitySystem/Attributes/RiftResourceAttributeSet.h"
#include "AbilitySystem/RiftGameplayTags.h"

UGE_GainResource::UGE_GainResource()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;

	FSetByCallerFloat UltimateChargeMagnitude;
	UltimateChargeMagnitude.DataTag = RiftGameplayTags::SetByCaller_UltimateCharge;

	FGameplayModifierInfo UltimateChargeModifier;
	UltimateChargeModifier.Attribute = URiftResourceAttributeSet::GetUltimateChargeAttribute();
	UltimateChargeModifier.ModifierOp = EGameplayModOp::Additive;
	UltimateChargeModifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(UltimateChargeMagnitude);

	Modifiers.Add(UltimateChargeModifier);
}
