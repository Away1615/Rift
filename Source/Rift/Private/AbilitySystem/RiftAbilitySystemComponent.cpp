// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/RiftAbilitySystemComponent.h"

URiftAbilitySystemComponent::URiftAbilitySystemComponent()
{
	SetIsReplicatedByDefault(true);
}

void URiftAbilitySystemComponent::AbilityInputTagPressed(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid()) return;

	bool bFoundActiveMatchingAbility = false;

	{
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
	}

	if (!bFoundActiveMatchingAbility)
	{
		FGameplayTagContainer ActivationTags;
		ActivationTags.AddTag(InputTag);
		TryActivateAbilitiesByTag(ActivationTags);
	}
}

void URiftAbilitySystemComponent::AbilityInputTagReleased(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid()) return;

	FScopedAbilityListLock AbilityListLock(*this);
	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if (!AbilitySpec.IsActive() ||
			!AbilitySpec.Ability ||
			!AbilitySpec.Ability->GetAssetTags().HasTagExact(InputTag))
		{
			continue;
		}

		AbilitySpecInputReleased(AbilitySpec);

		if (!IsOwnerActorAuthoritative() && AbilitySpec.Ability->bReplicateInputDirectly)
		{
			ServerSetInputReleased(AbilitySpec.Handle);
		}
	}
}
