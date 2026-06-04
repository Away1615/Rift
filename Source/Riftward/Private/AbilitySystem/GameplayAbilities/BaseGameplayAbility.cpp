// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/GameplayAbilities/BaseGameplayAbility.h"
#include "AbilitySystemComponent.h"
#include "Data/Player/Input/AbilityInputID.h"
#include "GameplayTags/RiftGameplayTags.h"

bool UBaseGameplayAbility::CanActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags,
	FGameplayTagContainer* OptionalRelevantTags) const
{

	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	const UAbilitySystemComponent* AbilitySystemComponent = ActorInfo
		? ActorInfo->AbilitySystemComponent.Get()
		: nullptr;

	if (!AbilitySystemComponent)
	{
		return false;
	}

	const FRiftGameplayTags& RiftTags = FRiftGameplayTags::Get();

	if (AbilitySystemComponent->HasMatchingGameplayTag(RiftTags.State_Ability_TwinSword_Core_DodgeActive))
	{
		return false;
	}

	const bool bComboActive =
		AbilitySystemComponent->HasMatchingGameplayTag(RiftTags.State_Ability_TwinSword_Primary_ComboActive)
		|| AbilitySystemComponent->HasMatchingGameplayTag(RiftTags.State_Ability_TwinSword_Secondary_ComboActive);

	if (bComboActive
		&& AbilityInputID != EAbilityInputID::Core)
	{
		return false;
	}

	return true;

}

FGameplayTag UBaseGameplayAbility::GetAbilityActiveStateTag() const
{
	return FGameplayTag();
}
