// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/RiftAbilitySystemComponent.h"

void URiftAbilitySystemComponent::AbilityInputTagPressed(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid()) return;

	bool bFoundActiveMatchingAbility = false;

	FScopedAbilityListLock AbilityListLock(*this);
	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if (!AbilitySpec.Ability || !AbilitySpec.Ability->GetAssetTags().HasTagExact(InputTag))
		{
			continue;
		}

		if (AbilitySpec.IsActive())
		{
			bFoundActiveMatchingAbility = true;
			AbilitySpecInputPressed(AbilitySpec);

			if (!IsOwnerActorAuthoritative() && AbilitySpec.Ability->bReplicateInputDirectly)
			{
				ServerSetInputPressed(AbilitySpec.Handle);
			}
		}
	}

	if (bFoundActiveMatchingAbility) return;

	FGameplayTagContainer ActivationTags;
	ActivationTags.AddTag(InputTag);
	TryActivateAbilitiesByTag(ActivationTags);
}
