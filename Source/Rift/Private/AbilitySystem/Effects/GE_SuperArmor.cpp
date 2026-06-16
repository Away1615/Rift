#include "AbilitySystem/Effects/GE_SuperArmor.h"

#include "AbilitySystem/RiftGameplayTags.h"
#include "GameplayEffectComponents/TargetTagsGameplayEffectComponent.h"

UGE_SuperArmor::UGE_SuperArmor()
{
	DurationPolicy = EGameplayEffectDurationType::HasDuration;
	DurationMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(0.35f));

	UTargetTagsGameplayEffectComponent* TargetTagsComponent =
		CreateDefaultSubobject<UTargetTagsGameplayEffectComponent>(TEXT("TargetTags"));
	FInheritedTagContainer GrantedTags;
	GrantedTags.AddTag(RiftGameplayTags::State_SuperArmor_Red);
	TargetTagsComponent->SetAndApplyTargetTagChanges(GrantedTags);
	GEComponents.Add(TargetTagsComponent);
}
