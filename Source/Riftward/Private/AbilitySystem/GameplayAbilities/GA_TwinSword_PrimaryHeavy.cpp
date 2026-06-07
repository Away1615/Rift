#include "AbilitySystem/GameplayAbilities/GA_TwinSword_PrimaryHeavy.h"

#include "Data/Player/Input/AbilityInputID.h"
#include "GameplayTags/RiftGameplayTags.h"

UGA_TwinSword_PrimaryHeavy::UGA_TwinSword_PrimaryHeavy()
{
	AbilityInputID = EAbilityInputID::PrimaryHeavy;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

FGameplayTag UGA_TwinSword_PrimaryHeavy::GetAbilityActiveStateTag() const
{
	return FRiftGameplayTags::Get().State_TwinSword_PrimaryHeavy_Active;
}

bool UGA_TwinSword_PrimaryHeavy::ShouldCommitComboAbility() const
{
	return true;
}
