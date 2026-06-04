// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/GameplayAbilities/GA_TwinSword_Core.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Character/PlayerCharacter.h"
#include "Data/Player/Ability/TwinSword/TwinSwordCoreAbilityConfig.h"
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

	const FGameplayAbilitySpec* Spec = GetCurrentAbilitySpec();
	const UTwinSwordCoreAbilityConfig* CoreConfig = Spec
		? Cast<UTwinSwordCoreAbilityConfig>(Spec->SourceObject.Get())
		: nullptr;

	if (!CoreConfig)
	{
		Logger::Error(PlayerCharacter, TEXT("TwinSword CoreAbilityConfig is missing"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UAnimMontage* MontageToPlay = CoreConfig->DodgeMontage;

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
	}

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
			1.0f,
			FName("Dodge"),
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

	ActiveMontage = nullptr;
	bPlayRecover = false;
	bAddedDodgeActiveTag = false;

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

FGameplayTag UGA_TwinSword_Core::GetAbilityActiveStateTag() const
{
	return FRiftGameplayTags::Get().State_Ability_TwinSword_Core_DodgeActive;
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
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UGA_TwinSword_Core::HandleMontageInterrupted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}
