// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/GameplayAbilities/BaseGameplayAbility.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/GameplayEffects/GE_DurationTag.h"
#include "AbilitySystem/GameplayEffects/GE_InfiniteTag.h"
#include "AbilitySystem/Attributes/ResourceAttributeSet.h"
#include "AbilitySystem/GameplayEffects/GE_ResourceCost.h"
#include "Data/Player/Ability/AbilityDefinitionConfig.h"
#include "Data/Player/Ability/Fragments/AbilityResourceCostFragment.h"
#include "Data/Player/Ability/PlayerAbilitySetConfig.h"
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

	const UAbilityDefinitionConfig* AbilityDefinition = GetAbilityDefinitionFromSpec(Handle, ActorInfo);
	if (!AbilityDefinition)
	{
		return true;
	}

	if (!AbilityDefinition->RequiredTags.IsEmpty()
		&& !AbilitySystemComponent->HasAllMatchingGameplayTags(AbilityDefinition->RequiredTags))
	{
		return false;
	}

	if (!AbilityDefinition->BlockedTags.IsEmpty()
		&& AbilitySystemComponent->HasAnyMatchingGameplayTags(AbilityDefinition->BlockedTags))
	{
		return false;
	}

	return true;

}

bool UBaseGameplayAbility::CheckCost(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CheckCost(Handle, ActorInfo, OptionalRelevantTags))
	{
		return false;
	}

	const UAbilityDefinitionConfig* AbilityDefinition = GetAbilityDefinitionFromSpec(Handle, ActorInfo);
	if (!AbilityDefinition)
	{
		return true;
	}

	const UAbilitySystemComponent* AbilitySystemComponent = ActorInfo
		? ActorInfo->AbilitySystemComponent.Get()
		: nullptr;

	if (!AbilitySystemComponent)
	{
		return false;
	}

	const UAbilityResourceCostFragment* Cost = AbilityDefinition->FindFragment<UAbilityResourceCostFragment>();
	if (!Cost)
	{
		return true;
	}

	if (Cost->Stamina > 0.0f
		&& AbilitySystemComponent->GetNumericAttribute(UResourceAttributeSet::GetStaminaAttribute()) < Cost->Stamina)
	{
		return false;
	}

	if (Cost->Mana > 0.0f
		&& AbilitySystemComponent->GetNumericAttribute(UResourceAttributeSet::GetManaAttribute()) < Cost->Mana)
	{
		return false;
	}

	return true;
}

void UBaseGameplayAbility::ApplyCost(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo) const
{
	const UAbilityDefinitionConfig* AbilityDefinition = GetAbilityDefinitionFromSpec(Handle, ActorInfo);
	if (!AbilityDefinition)
	{
		Super::ApplyCost(Handle, ActorInfo, ActivationInfo);
		return;
	}

	const UAbilityResourceCostFragment* Cost = AbilityDefinition->FindFragment<UAbilityResourceCostFragment>();
	if (!Cost || (Cost->Stamina <= 0.0f && Cost->Mana <= 0.0f))
	{
		Super::ApplyCost(Handle, ActorInfo, ActivationInfo);
		return;
	}

	TSubclassOf<UGameplayEffect> CostEffectClass = Cost->CostEffectClass;
	if (!CostEffectClass)
	{
		CostEffectClass = UGE_ResourceCost::StaticClass();
	}

	FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(
		Handle,
		ActorInfo,
		ActivationInfo,
		CostEffectClass,
		GetAbilityLevel(Handle, ActorInfo)
	);

	if (!SpecHandle.IsValid())
	{
		return;
	}

	SpecHandle.Data->SetSetByCallerMagnitude(
		FRiftGameplayTags::Get().Data_StaminaCost,
		-Cost->Stamina
	);

	SpecHandle.Data->SetSetByCallerMagnitude(
		FRiftGameplayTags::Get().Data_ManaCost,
		-Cost->Mana
	);

	ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, SpecHandle);
}

FActiveGameplayEffectHandle UBaseGameplayAbility::ApplyInfiniteStateTagEffect(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayTag StateTag) const
{
	return ApplyStateTagEffect(
		Handle,
		ActorInfo,
		ActivationInfo,
		UGE_InfiniteTag::StaticClass(),
		StateTag
	);
}

FActiveGameplayEffectHandle UBaseGameplayAbility::ApplyDurationStateTagEffect(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayTag StateTag,
	const float Duration) const
{
	if (Duration <= 0.0f)
	{
		return FActiveGameplayEffectHandle();
	}

	return ApplyStateTagEffect(
		Handle,
		ActorInfo,
		ActivationInfo,
		UGE_DurationTag::StaticClass(),
		StateTag,
		Duration
	);
}

void UBaseGameplayAbility::RemoveGrantedStateTagEffect(FActiveGameplayEffectHandle& EffectHandle) const
{
	if (!EffectHandle.IsValid())
	{
		return;
	}

	if (UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo())
	{
		AbilitySystemComponent->RemoveActiveGameplayEffect(EffectHandle);
	}

	EffectHandle.Invalidate();
}

FActiveGameplayEffectHandle UBaseGameplayAbility::ApplyStateTagEffect(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const TSubclassOf<UGameplayEffect> EffectClass,
	const FGameplayTag StateTag,
	const float Duration) const
{
	if (!EffectClass || !StateTag.IsValid())
	{
		return FActiveGameplayEffectHandle();
	}

	FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(
		Handle,
		ActorInfo,
		ActivationInfo,
		EffectClass,
		GetAbilityLevel(Handle, ActorInfo)
	);

	if (!SpecHandle.IsValid())
	{
		return FActiveGameplayEffectHandle();
	}

	if (Duration > 0.0f)
	{
		SpecHandle.Data->SetSetByCallerMagnitude(
			FRiftGameplayTags::Get().Data_Duration,
			Duration
		);
	}

	SpecHandle.Data->DynamicGrantedTags.AddTag(StateTag);
	return ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, SpecHandle);
}

FGameplayTag UBaseGameplayAbility::GetAbilityActiveStateTag() const
{
	const UAbilityDefinitionConfig* AbilityDefinition = GetAbilityDefinition();
	return AbilityDefinition ? AbilityDefinition->ActiveStateTag : FGameplayTag();
}

const UAbilityDefinitionConfig* UBaseGameplayAbility::GetAbilityDefinition() const
{
	return GetAbilityDefinitionFromSpec(CurrentSpecHandle, CurrentActorInfo);
}

const UAbilityDefinitionConfig* UBaseGameplayAbility::GetAbilityDefinitionFromSpec(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo) const
{
	const UAbilitySystemComponent* AbilitySystemComponent = ActorInfo
		? ActorInfo->AbilitySystemComponent.Get()
		: nullptr;

	const FGameplayAbilitySpec* Spec = AbilitySystemComponent
		? AbilitySystemComponent->FindAbilitySpecFromHandle(Handle)
		: nullptr;

	if (!Spec)
	{
		return nullptr;
	}

	if (const UAbilityDefinitionConfig* AbilityDefinition = Cast<UAbilityDefinitionConfig>(Spec->SourceObject.Get()))
	{
		return AbilityDefinition;
	}

	const UPlayerAbilitySetConfig* AbilitySetConfig = GetAbilitySetConfigFromSpec(Handle, ActorInfo);
	if (!AbilitySetConfig)
	{
		return nullptr;
	}

	TArray<FGameplayTag> DynamicTags;
	Spec->GetDynamicSpecSourceTags().GetGameplayTagArray(DynamicTags);
	for (const FGameplayTag& DynamicTag : DynamicTags)
	{
		if (const UAbilityDefinitionConfig* AbilityDefinition = AbilitySetConfig->FindAbilityByID(DynamicTag))
		{
			return AbilityDefinition;
		}
	}

	return AbilitySetConfig->FindAbilityByInputID(static_cast<EAbilityInputID>(Spec->InputID));
}

const UPlayerAbilitySetConfig* UBaseGameplayAbility::GetAbilitySetConfigFromSpec(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo) const
{
	const UAbilitySystemComponent* AbilitySystemComponent = ActorInfo
		? ActorInfo->AbilitySystemComponent.Get()
		: nullptr;

	const FGameplayAbilitySpec* Spec = AbilitySystemComponent
		? AbilitySystemComponent->FindAbilitySpecFromHandle(Handle)
		: nullptr;

	return Spec
		? Cast<UPlayerAbilitySetConfig>(Spec->SourceObject.Get())
		: nullptr;
}
