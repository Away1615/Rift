// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/GameplayAbilities/GA_PlayMontage.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Data/Player/Ability/AbilityDefinitionConfig.h"
#include "Data/Player/Ability/Fragments/AbilityMontageSectionsFragment.h"

bool UMontageGameplayAbility::OnAbilityMontageCompleted()
{
	return bEndAbilityOnCurrentMontage;
}

void UMontageGameplayAbility::OnAbilityMontageBlendedIn()
{
}

void UMontageGameplayAbility::OnAbilityMontageBlendOut()
{
}

bool UMontageGameplayAbility::OnAbilityMontageCancelled()
{
	return bEndAbilityOnCurrentMontage;
}

bool UMontageGameplayAbility::OnAbilityMontageInterrupted()
{
	return bEndAbilityOnCurrentMontage;
}

bool UMontageGameplayAbility::TryPlayMontage(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	UAnimMontage* MontageToPlay,
	const float PlayRate,
	const FName StartSection,
	const bool bEndAbilityOnFinish)
{
	if (!MontageToPlay) return false;

	UAbilityTask_PlayMontageAndWait* MontageTask =
		UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this,
			NAME_None,
			MontageToPlay,
			PlayRate,
			StartSection,
			true
		);

	if (!MontageTask) return false;

	bEndAbilityOnCurrentMontage = bEndAbilityOnFinish;
	MontageTask->OnCompleted.AddDynamic(this, &UMontageGameplayAbility::HandleMontageCompleted);
	MontageTask->OnBlendedIn.AddDynamic(this, &UMontageGameplayAbility::HandleMontageBlendedIn);
	MontageTask->OnBlendOut.AddDynamic(this, &UMontageGameplayAbility::HandleMontageBlendOut);
	MontageTask->OnCancelled.AddDynamic(this, &UMontageGameplayAbility::HandleMontageCancelled);
	MontageTask->OnInterrupted.AddDynamic(this, &UMontageGameplayAbility::HandleMontageInterrupted);
	MontageTask->ReadyForActivation();
	return true;
}

void UMontageGameplayAbility::HandleMontageCompleted()
{
	if (OnAbilityMontageCompleted())
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	}
}

void UMontageGameplayAbility::HandleMontageBlendedIn()
{
	OnAbilityMontageBlendedIn();
}

void UMontageGameplayAbility::HandleMontageBlendOut()
{
	OnAbilityMontageBlendOut();
}

void UMontageGameplayAbility::HandleMontageCancelled()
{
	if (OnAbilityMontageCancelled())
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
	}
}

void UMontageGameplayAbility::HandleMontageInterrupted()
{
	if (OnAbilityMontageInterrupted())
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
	}
}

void UGA_PlayMontage::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	const UAbilityDefinitionConfig* AbilityDefinition = GetAbilityDefinition();
	const UAbilityMontageSectionsFragment* SectionsFragment = AbilityDefinition
		? AbilityDefinition->FindFragment<UAbilityMontageSectionsFragment>()
		: nullptr;

	if (!SectionsFragment || SectionsFragment->Sections.IsEmpty())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	const FAbilityMontageSection& Step = SectionsFragment->Sections[0];
	UAnimMontage* MontageToPlay = SectionsFragment->Montage ? SectionsFragment->Montage.Get() : Step.Montage.Get();
	if (!MontageToPlay)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!TryPlayMontage(Handle, ActorInfo, MontageToPlay, Step.PlayRate, Step.SectionName, true))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
	}
}
