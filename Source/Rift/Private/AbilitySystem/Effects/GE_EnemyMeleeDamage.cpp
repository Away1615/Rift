#include "AbilitySystem/Effects/GE_EnemyMeleeDamage.h"

#include "AbilitySystem/Attributes/RiftPlayerAttributeSet.h"
#include "AbilitySystem/RiftGameplayTags.h"
#include "GameplayEffectComponents/TargetTagRequirementsGameplayEffectComponent.h"

UGE_EnemyMeleeDamage::UGE_EnemyMeleeDamage()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;

	UTargetTagRequirementsGameplayEffectComponent* TargetTagRequirementsComponent =
		CreateDefaultSubobject<UTargetTagRequirementsGameplayEffectComponent>(TEXT("TargetTagRequirements"));
	TargetTagRequirementsComponent->ApplicationTagRequirements.IgnoreTags.AddTag(RiftGameplayTags::State_Invincible);
	GEComponents.Add(TargetTagRequirementsComponent);

	FSetByCallerFloat DamageMagnitude;
	DamageMagnitude.DataTag = RiftGameplayTags::SetByCaller_Damage;

	FGameplayModifierInfo DamageModifier;
	DamageModifier.Attribute = URiftPlayerAttributeSet::GetDamageAttribute();
	DamageModifier.ModifierOp = EGameplayModOp::Additive;
	DamageModifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(DamageMagnitude);

	Modifiers.Add(DamageModifier);

	FSetByCallerFloat PoiseDamageMagnitude;
	PoiseDamageMagnitude.DataTag = RiftGameplayTags::SetByCaller_PoiseDamage;

	FGameplayModifierInfo PoiseDamageModifier;
	PoiseDamageModifier.Attribute = URiftPlayerAttributeSet::GetPoiseDamageAttribute();
	PoiseDamageModifier.ModifierOp = EGameplayModOp::Additive;
	PoiseDamageModifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(PoiseDamageMagnitude);

	Modifiers.Add(PoiseDamageModifier);
}
