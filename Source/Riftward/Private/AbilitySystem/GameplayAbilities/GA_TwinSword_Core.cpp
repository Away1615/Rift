// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/GameplayAbilities/GA_TwinSword_Core.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Character/PlayerCharacter.h"
#include "Data/Player/Ability/PlayerAbilitySetConfig.h"
#include "Data/Player/Input/AbilityInputID.h"
#include "Debug/Logger.h"
#include "GameplayTags/RiftGameplayTags.h"

UGA_TwinSword_Core::UGA_TwinSword_Core()
{
	AbilityInputID = EAbilityInputID::Core;

	// A skill instance, which can save the skill running status
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	// Executed locally first, then confirmed by the server
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
 }

void UGA_TwinSword_Core::InputReleased(const FGameplayAbilitySpecHandle Handle,
                                       const FGameplayAbilityActorInfo* ActorInfo,
                                       const FGameplayAbilityActivationInfo ActivationInfo)
{
	Super::InputReleased(Handle, ActorInfo, ActivationInfo);
}

void UGA_TwinSword_Core::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                         const FGameplayAbilityActorInfo* ActorInfo,
                                         const FGameplayAbilityActivationInfo ActivationInfo,
                                         const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(GetAvatarActorFromActorInfo());
	if (!PlayerCharacter)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	const FPlayerAbilityEntry* CoreEntry = GetAbilityEntry();
	const URiftAbilityMontageFragment* MontageFragment = CoreEntry
		? CoreEntry->FindFragment<URiftAbilityMontageFragment>()
		: nullptr;
	const URiftAbilityDodgeFragment* DodgeFragment = CoreEntry
		? CoreEntry->FindFragment<URiftAbilityDodgeFragment>()
		: nullptr;
	if (!CoreEntry || !MontageFragment || !DodgeFragment)
	{
		Logger::Error(PlayerCharacter, TEXT("TwinSword core fragments are missing"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UAnimMontage* MontageToPlay = MontageFragment->Montage;

	if (!MontageToPlay)
	{
		Logger::Error(PlayerCharacter, TEXT("TwinSword dodge montage is missing"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		Logger::Error(PlayerCharacter, TEXT("TwinSword dodge montage CommitAbility failed"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	ActiveMontage = MontageToPlay;
	bPlayRecover = !PlayerCharacter->IsMovementAccelerating();

	const FRiftGameplayTags& RiftTags = FRiftGameplayTags::Get();
	if (UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo())
	{
		AbilitySystemComponent->AddLooseGameplayTag(GetAbilityActiveStateTag());
		bAddedDodgeActiveTag = true;

		if (DodgeFragment->PerfectWindowDuration > 0.0f)
		{
			AbilitySystemComponent->AddLooseGameplayTag(RiftTags.State_Ability_TwinSword_Core_PerfectDodgeWindow);
			bAddedPerfectWindowTag = true;
		}
	}

	if (DodgeFragment->PerfectWindowDuration > 0.0f)
	{
		UAbilityTask_WaitDelay* PerfectWindowTask =
			UAbilityTask_WaitDelay::WaitDelay(this, DodgeFragment->PerfectWindowDuration);
		PerfectWindowTask->OnFinish.AddDynamic(this, &UGA_TwinSword_Core::HandlePerfectWindowExpired);
		PerfectWindowTask->ReadyForActivation();
	}

	UAbilityTask_WaitGameplayEvent* PerfectDodgeSuccessTask =
		UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
			this,
			RiftTags.Event_Ability_TwinSword_Core_PerfectDodgeSuccess,
			nullptr,
			false,
			true
		);

	PerfectDodgeSuccessTask->EventReceived.AddDynamic(
		this,
		&UGA_TwinSword_Core::HandlePerfectDodgeSuccess
	);

	PerfectDodgeSuccessTask->ReadyForActivation();

	UAbilityTask_WaitGameplayEvent* DodgeFinishedTask =
		UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
			this,
			RiftTags.Event_Ability_TwinSword_Core_DodgeFinished,
			nullptr,
			false,
			true
		);

	DodgeFinishedTask->EventReceived.AddDynamic(
		this,
		&UGA_TwinSword_Core::HandleDodgeFinished
	);

	DodgeFinishedTask->ReadyForActivation();

	UAbilityTask_PlayMontageAndWait* MontageTask =
		UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this,
			NAME_None,
			MontageToPlay,
			MontageFragment->PlayRate,
			MontageFragment->StartSection.IsNone() ? FName(TEXT("Dodge")) : MontageFragment->StartSection,
			true
		);

	MontageTask->OnCompleted.AddDynamic(this, &UGA_TwinSword_Core::HandleMontageCompleted);
	MontageTask->OnCancelled.AddDynamic(this, &UGA_TwinSword_Core::HandleMontageCancelled);
	MontageTask->OnInterrupted.AddDynamic(this, &UGA_TwinSword_Core::HandleMontageInterrupted);

	MontageTask->ReadyForActivation();
}

void UGA_TwinSword_Core::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (bAddedDodgeActiveTag)
	{
		if (UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo())
		{
			AbilitySystemComponent->RemoveLooseGameplayTag(GetAbilityActiveStateTag());
		}
	}

	if (bAddedPerfectWindowTag)
	{
		if (UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo())
		{
			AbilitySystemComponent->RemoveLooseGameplayTag(FRiftGameplayTags::Get().State_Ability_TwinSword_Core_PerfectDodgeWindow);
		}
	}

	ActiveMontage = nullptr;
	bPlayRecover = false;
	bAddedDodgeActiveTag = false;
	bAddedPerfectWindowTag = false;
	bChangingToPerfectDodgeMontage = false;

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

FGameplayTag UGA_TwinSword_Core::GetAbilityActiveStateTag() const
{
	return FRiftGameplayTags::Get().State_Ability_TwinSword_Core_DodgeActive;
}

void UGA_TwinSword_Core::HandlePerfectDodgeSuccess(FGameplayEventData Payload)
{
	UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo();
	if (!AbilitySystemComponent) return;

	const FRiftGameplayTags& RiftTags = FRiftGameplayTags::Get();
	if (!AbilitySystemComponent->HasMatchingGameplayTag(RiftTags.State_Ability_TwinSword_Core_PerfectDodgeWindow))
	{
		return;
	}

	AbilitySystemComponent->AddLooseGameplayTag(RiftTags.State_Ability_TwinSword_Core_PerfectDodgeEmpowered);
	AbilitySystemComponent->RemoveLooseGameplayTag(RiftTags.State_Ability_TwinSword_Core_PerfectDodgeWindow);
	bAddedPerfectWindowTag = false;

	const FPlayerAbilityEntry* CoreEntry = GetAbilityEntry();
	const URiftAbilityDodgeFragment* DodgeFragment = CoreEntry
		? CoreEntry->FindFragment<URiftAbilityDodgeFragment>()
		: nullptr;
	PlayPerfectDodgeFeedback(DodgeFragment);
	PlayPerfectDodgeMontage(DodgeFragment);
}

void UGA_TwinSword_Core::HandlePerfectWindowExpired()
{
	if (!bAddedPerfectWindowTag) return;

	if (UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo())
	{
		AbilitySystemComponent->RemoveLooseGameplayTag(FRiftGameplayTags::Get().State_Ability_TwinSword_Core_PerfectDodgeWindow);
	}

	bAddedPerfectWindowTag = false;
}

void UGA_TwinSword_Core::PlayPerfectDodgeFeedback(const URiftAbilityDodgeFragment* DodgeFragment)
{
	if (!DodgeFragment || !CurrentActorInfo || !CurrentActorInfo->IsNetAuthority()) return;

	APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(GetAvatarActorFromActorInfo());
	if (!PlayerCharacter) return;

	PlayerCharacter->MulticastPlayAbilityCue(
		DodgeFragment->PerfectSuccessEffect,
		DodgeFragment->PerfectSuccessSound,
		DodgeFragment->CueSocketName,
		DodgeFragment->CueLocationOffset
	);
}

void UGA_TwinSword_Core::PlayPerfectDodgeMontage(const URiftAbilityDodgeFragment* DodgeFragment)
{
	if (!DodgeFragment) return;

	UAnimInstance* AnimInstance = CurrentActorInfo
		? CurrentActorInfo->GetAnimInstance()
		: nullptr;
	if (!AnimInstance) return;

	if (!DodgeFragment->PerfectSuccessSection.IsNone() && ActiveMontage)
	{
		AnimInstance->Montage_JumpToSection(DodgeFragment->PerfectSuccessSection, ActiveMontage);
		return;
	}

	if (!DodgeFragment->PerfectSuccessMontage) return;

	ActiveMontage = DodgeFragment->PerfectSuccessMontage;
	bChangingToPerfectDodgeMontage = true;

	UAbilityTask_PlayMontageAndWait* MontageTask =
		UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this,
			NAME_None,
			DodgeFragment->PerfectSuccessMontage,
			DodgeFragment->PerfectSuccessPlayRate,
			NAME_None,
			true
		);

	MontageTask->OnCompleted.AddDynamic(this, &UGA_TwinSword_Core::HandleMontageCompleted);
	MontageTask->OnCancelled.AddDynamic(this, &UGA_TwinSword_Core::HandleMontageCancelled);
	MontageTask->OnInterrupted.AddDynamic(this, &UGA_TwinSword_Core::HandleMontageInterrupted);
	MontageTask->ReadyForActivation();
}

void UGA_TwinSword_Core::HandleDodgeFinished(FGameplayEventData Payload)
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
		AnimInstance->Montage_JumpToSection(FName("Recover"), ActiveMontage);
		return;
	}

	AnimInstance->Montage_Stop(0.08f, ActiveMontage);
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_TwinSword_Core::HandleMontageCompleted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_TwinSword_Core::HandleMontageCancelled()
{
	if (bChangingToPerfectDodgeMontage)
	{
		bChangingToPerfectDodgeMontage = false;
		return;
	}

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UGA_TwinSword_Core::HandleMontageInterrupted()
{
	if (bChangingToPerfectDodgeMontage)
	{
		bChangingToPerfectDodgeMontage = false;
		return;
	}

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}
