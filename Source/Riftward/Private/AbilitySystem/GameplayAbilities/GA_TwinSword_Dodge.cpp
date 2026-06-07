// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/GameplayAbilities/GA_TwinSword_Dodge.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Character/PlayerCharacter.h"
#include "Data/Player/Ability/AbilityDefinitionConfig.h"
#include "Data/Player/Ability/Fragments/AbilityPerfectDodgeFragment.h"
#include "Data/Player/Ability/Fragments/AbilityMontageSectionsFragment.h"
#include "Data/Player/Input/AbilityInputID.h"
#include "Debug/Logger.h"
#include "GameplayTags/RiftGameplayTags.h"

UGA_TwinSword_Dodge::UGA_TwinSword_Dodge()
{
	AbilityInputID = EAbilityInputID::Core;

	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

void UGA_TwinSword_Dodge::InputReleased(const FGameplayAbilitySpecHandle Handle,
                                        const FGameplayAbilityActorInfo* ActorInfo,
                                        const FGameplayAbilityActivationInfo ActivationInfo)
{
	Super::InputReleased(Handle, ActorInfo, ActivationInfo);
}

void UGA_TwinSword_Dodge::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                          const FGameplayAbilityActorInfo* ActorInfo,
                                          const FGameplayAbilityActivationInfo ActivationInfo,
                                          const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	const APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(GetAvatarActorFromActorInfo());
	if (!PlayerCharacter)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	const UAbilityDefinitionConfig* DodgeDefinition = GetAbilityDefinition();
	const UAbilityMontageSectionsFragment* SectionsFragment = DodgeDefinition
		? DodgeDefinition->FindFragment<UAbilityMontageSectionsFragment>()
		: nullptr;
	const UAbilityPerfectDodgeFragment* PerfectDodgeFragment = DodgeDefinition
		? DodgeDefinition->FindFragment<UAbilityPerfectDodgeFragment>()
		: nullptr;
	if (!DodgeDefinition || !SectionsFragment || SectionsFragment->Sections.IsEmpty() || !PerfectDodgeFragment)
	{
		Logger::Error(PlayerCharacter, TEXT("TwinSword dodge fragments are missing"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	const FAbilityMontageSection& DodgeSection = SectionsFragment->Sections[0];
	UAnimMontage* DodgeMontage = SectionsFragment->Montage ? SectionsFragment->Montage.Get() : DodgeSection.Montage.Get();
	if (!DodgeMontage)
	{
		Logger::Error(PlayerCharacter, TEXT("TwinSword dodge montage is missing"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		Logger::Error(PlayerCharacter, TEXT("TwinSword dodge CommitAbility failed"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	ActiveMontage = DodgeMontage;
	bPlayRecover = !PlayerCharacter->IsMovementAccelerating();

	const FRiftGameplayTags& RiftTags = FRiftGameplayTags::Get();
	DodgeActiveEffectHandle = ApplyInfiniteStateTagEffect(
		Handle,
		ActorInfo,
		ActivationInfo,
		GetAbilityActiveStateTag()
	);

	UAbilityTask_WaitGameplayEvent* PerfectDodgeSuccessTask =
		UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
			this,
			RiftTags.Event_TwinSword_Dodge_PerfectSuccess,
			nullptr,
			false,
			true
		);
	PerfectDodgeSuccessTask->EventReceived.AddDynamic(this, &UGA_TwinSword_Dodge::HandlePerfectDodgeSuccess);
	PerfectDodgeSuccessTask->ReadyForActivation();

	const FGameplayTag DodgeFinishedTag = DodgeSection.ChainPointEventTag.IsValid()
		? DodgeSection.ChainPointEventTag
		: RiftTags.Event_TwinSword_Dodge_Finished;

	UAbilityTask_WaitGameplayEvent* DodgeFinishedTask =
		UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
			this,
			DodgeFinishedTag,
			nullptr,
			false,
			true
		);

	DodgeFinishedTask->EventReceived.AddDynamic(
		this,
		&UGA_TwinSword_Dodge::HandleDodgeFinished
	);

	DodgeFinishedTask->ReadyForActivation();

	if (!TryPlayMontage(
		Handle,
		ActorInfo,
		DodgeMontage,
		DodgeSection.PlayRate,
		DodgeSection.SectionName,
		true
	))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
	}
}

void UGA_TwinSword_Dodge::EndAbility(const FGameplayAbilitySpecHandle Handle,
                                     const FGameplayAbilityActorInfo* ActorInfo,
                                     const FGameplayAbilityActivationInfo ActivationInfo,
                                     bool bReplicateEndAbility,
                                     bool bWasCancelled)
{
	RemoveGrantedStateTagEffect(DodgeActiveEffectHandle);

	ActiveMontage = nullptr;
	bPlayRecover = false;
	bChangingToPerfectDodgeMontage = false;

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

FGameplayTag UGA_TwinSword_Dodge::GetAbilityActiveStateTag() const
{
	return FRiftGameplayTags::Get().State_TwinSword_Dodge_Active;
}

void UGA_TwinSword_Dodge::HandlePerfectDodgeSuccess(FGameplayEventData Payload)
{
	const UAbilityDefinitionConfig* DodgeDefinition = GetAbilityDefinition();
	const UAbilityPerfectDodgeFragment* PerfectDodgeFragment = DodgeDefinition
		? DodgeDefinition->FindFragment<UAbilityPerfectDodgeFragment>()
		: nullptr;

	const FRiftGameplayTags& RiftTags = FRiftGameplayTags::Get();
	const FGameplayTag RewardStateTag = PerfectDodgeFragment && PerfectDodgeFragment->PerfectSuccessStateTag.IsValid()
		? PerfectDodgeFragment->PerfectSuccessStateTag
		: RiftTags.State_TwinSword_Dodge_Heal;
	const float RewardDuration = PerfectDodgeFragment
		? PerfectDodgeFragment->PerfectSuccessStateDuration
		: 0.0f;

	ApplyDurationStateTagEffect(
		CurrentSpecHandle,
		CurrentActorInfo,
		CurrentActivationInfo,
		RewardStateTag,
		RewardDuration
	);

	PlayPerfectDodgeFeedback(PerfectDodgeFragment);
	PlayPerfectDodgeMontage(PerfectDodgeFragment);
}

void UGA_TwinSword_Dodge::PlayPerfectDodgeFeedback(const UAbilityPerfectDodgeFragment* PerfectDodgeFragment)
{
	if (!PerfectDodgeFragment || !CurrentActorInfo || !CurrentActorInfo->IsNetAuthority()) return;

	APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(GetAvatarActorFromActorInfo());
	if (!PlayerCharacter) return;

	PlayerCharacter->MulticastPlayAbilityCue(
		PerfectDodgeFragment->PerfectSuccessEffect,
		PerfectDodgeFragment->PerfectSuccessSound,
		PerfectDodgeFragment->CueSocketName,
		PerfectDodgeFragment->CueLocationOffset
	);
}

void UGA_TwinSword_Dodge::PlayPerfectDodgeMontage(const UAbilityPerfectDodgeFragment* PerfectDodgeFragment)
{
	if (!PerfectDodgeFragment) return;

	UAnimInstance* AnimInstance = CurrentActorInfo
		? CurrentActorInfo->GetAnimInstance()
		: nullptr;
	if (!AnimInstance) return;

	if (!PerfectDodgeFragment->PerfectSuccessSection.IsNone() && ActiveMontage)
	{
		AnimInstance->Montage_JumpToSection(PerfectDodgeFragment->PerfectSuccessSection, ActiveMontage);
		return;
	}

	if (!PerfectDodgeFragment->PerfectSuccessMontage) return;

	ActiveMontage = PerfectDodgeFragment->PerfectSuccessMontage;
	bChangingToPerfectDodgeMontage = true;

	if (!TryPlayMontage(
		CurrentSpecHandle,
		CurrentActorInfo,
		PerfectDodgeFragment->PerfectSuccessMontage,
		PerfectDodgeFragment->PerfectSuccessPlayRate,
		NAME_None,
		true
	))
	{
		bChangingToPerfectDodgeMontage = false;
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
	}
}

void UGA_TwinSword_Dodge::HandleDodgeFinished(FGameplayEventData Payload)
{
	UAnimInstance* AnimInstance = CurrentActorInfo
		? CurrentActorInfo->GetAnimInstance()
		: nullptr;

	if (!AnimInstance || !ActiveMontage)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	if (bPlayRecover)
	{
		bPlayRecover = false;
		AnimInstance->Montage_JumpToSection(FName("Recover"), ActiveMontage);
		return;
	}

	AnimInstance->Montage_Stop(0.08f, ActiveMontage);
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

bool UGA_TwinSword_Dodge::OnAbilityMontageCancelled()
{
	if (bChangingToPerfectDodgeMontage)
	{
		bChangingToPerfectDodgeMontage = false;
		return false;
	}

	return true;
}

bool UGA_TwinSword_Dodge::OnAbilityMontageInterrupted()
{
	if (bChangingToPerfectDodgeMontage)
	{
		bChangingToPerfectDodgeMontage = false;
		return false;
	}

	return true;
}
