// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/GameplayAbilities/BaseGameplayAbility.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/Attributes/ResourceAttributeSet.h"
#include "AbilitySystem/GameplayEffects/GE_ResourceCost.h"
#include "Data/Player/Input/AbilityInputID.h"
#include "Data/Player/Ability/PlayerAbilitySetConfig.h"
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

	const FPlayerAbilityEntry* AbilityEntry = GetAbilityEntryFromSpec(Handle, ActorInfo);
	if (!AbilityEntry)
	{
		return true;
	}

	if (!AbilityEntry->RequiredTags.IsEmpty()
		&& !AbilitySystemComponent->HasAllMatchingGameplayTags(AbilityEntry->RequiredTags))
	{
		return false;
	}

	if (!AbilityEntry->BlockedTags.IsEmpty()
		&& AbilitySystemComponent->HasAnyMatchingGameplayTags(AbilityEntry->BlockedTags))
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

	const FPlayerAbilityEntry* AbilityEntry = GetAbilityEntryFromSpec(Handle, ActorInfo);
	if (!AbilityEntry)
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

	const URiftAbilityCostFragment* Cost = AbilityEntry->FindFragment<URiftAbilityCostFragment>();
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
	const FPlayerAbilityEntry* AbilityEntry = GetAbilityEntryFromSpec(Handle, ActorInfo);
	if (!AbilityEntry)
	{
		Super::ApplyCost(Handle, ActorInfo, ActivationInfo);
		return;
	}

	const URiftAbilityCostFragment* Cost = AbilityEntry->FindFragment<URiftAbilityCostFragment>();
	if (!Cost || (Cost->Stamina <= 0.0f && Cost->Mana <= 0.0f))
	{
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

FGameplayTag UBaseGameplayAbility::GetAbilityActiveStateTag() const
{
	return FGameplayTag();
}

const FPlayerAbilityEntry* UBaseGameplayAbility::GetAbilityEntry() const
{
	return GetAbilityEntryFromSpec(CurrentSpecHandle, CurrentActorInfo);
}

const FPlayerAbilityEntry* UBaseGameplayAbility::GetAbilityEntryFromSpec(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo) const
{
	const UAbilitySystemComponent* AbilitySystemComponent = ActorInfo
		? ActorInfo->AbilitySystemComponent.Get()
		: nullptr;

	const FGameplayAbilitySpec* Spec = AbilitySystemComponent
		? AbilitySystemComponent->FindAbilitySpecFromHandle(Handle)
		: nullptr;

	const UPlayerAbilitySetConfig* AbilitySetConfig = GetAbilitySetConfigFromSpec(Handle, ActorInfo);
	if (!Spec || !AbilitySetConfig)
	{
		return nullptr;
	}

	TArray<FGameplayTag> DynamicTags;
	Spec->GetDynamicSpecSourceTags().GetGameplayTagArray(DynamicTags);
	for (const FGameplayTag& DynamicTag : DynamicTags)
	{
		if (const FPlayerAbilityEntry* AbilityEntry = AbilitySetConfig->FindAbilityByID(DynamicTag))
		{
			return AbilityEntry;
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
