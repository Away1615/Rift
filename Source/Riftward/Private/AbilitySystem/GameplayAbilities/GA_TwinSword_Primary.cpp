// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/GameplayAbilities/GA_TwinSword_Primary.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Abilities/Tasks/AbilityTask_WaitInputPress.h"
#include "Animation/AnimInstance.h"
#include "Character/PlayerCharacter.h"
#include "Data/Ability/TwinSword/TwinSwordPrimaryAbilityConfig.h"
#include "Data/Models/AbilityInputID.h"
#include "Debug/Logger.h"
#include "GameplayTags/RiftGameplayTags.h"

UGA_TwinSword_Primary::UGA_TwinSword_Primary()
{
	AbilityInputID = EAbilityInputID::Primary;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

void UGA_TwinSword_Primary::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
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
	const UTwinSwordPrimaryAbilityConfig* PrimaryConfig = Spec
		? Cast<UTwinSwordPrimaryAbilityConfig>(Spec->SourceObject.Get())
		: nullptr;

	if (!PrimaryConfig)
	{
		Logger::Error(PlayerCharacter, TEXT("TwinSword PrimaryAbilityConfig is missing"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!PrimaryConfig->AttackMontage)
	{
		Logger::Error(PlayerCharacter, TEXT("TwinSword attack montage is missing"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (PrimaryConfig->ComboSections.IsEmpty())
	{
		Logger::Error(PlayerCharacter, TEXT("TwinSword combo sections are missing"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	ActiveMontage = PrimaryConfig->AttackMontage;
	ActiveComboSections = PrimaryConfig->ComboSections;
	bHasBufferedPrimaryInput = false;
	bCanConsumeBufferedPrimaryInput = false;

	UAbilityTask_PlayMontageAndWait* MontageTask =
		UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this,
			NAME_None,
			ActiveMontage,
			1.0f,
			ActiveComboSections[0],
			true
		);

	MontageTask->OnCompleted.AddDynamic(this, &UGA_TwinSword_Primary::HandleMontageCompleted);
	MontageTask->OnCancelled.AddDynamic(this, &UGA_TwinSword_Primary::HandleMontageCancelled);
	MontageTask->OnInterrupted.AddDynamic(this, &UGA_TwinSword_Primary::HandleMontageInterrupted);
	MontageTask->ReadyForActivation();

	ResetComboSectionLinks();
	WaitForNextPrimaryInput();
	WaitForComboWindow();
}

void UGA_TwinSword_Primary::EndAbility(const FGameplayAbilitySpecHandle Handle,
                                       const FGameplayAbilityActorInfo* ActorInfo,
                                       const FGameplayAbilityActivationInfo ActivationInfo,
                                       bool bReplicateEndAbility,
                                       bool bWasCancelled)
{
	ActiveMontage = nullptr;
	ActiveComboSections.Empty();
	bHasBufferedPrimaryInput = false;
	bCanConsumeBufferedPrimaryInput = false;

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_TwinSword_Primary::ResetComboSectionLinks() const
{
	UAnimInstance* AnimInstance = CurrentActorInfo
		? CurrentActorInfo->GetAnimInstance()
		: nullptr;

	if (!AnimInstance || !ActiveMontage) return;

	for (const FName ComboSection : ActiveComboSections)
	{
		AnimInstance->Montage_SetNextSection(ComboSection, NAME_None, ActiveMontage);
	}
}

void UGA_TwinSword_Primary::WaitForNextPrimaryInput()
{
	UAbilityTask_WaitInputPress* InputTask =
		UAbilityTask_WaitInputPress::WaitInputPress(this, false);

	InputTask->OnPress.AddDynamic(this, &UGA_TwinSword_Primary::HandleInputPressed);
	InputTask->ReadyForActivation();
}

void UGA_TwinSword_Primary::WaitForComboWindow()
{
	const FRiftGameplayTags& RiftTags = FRiftGameplayTags::Get();

	UAbilityTask_WaitGameplayEvent* ComboWindowTask =
		UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
			this,
			RiftTags.Event_Ability_TwinSword_Primary_ComboWindow,
			nullptr,
			false,
			true
		);

	ComboWindowTask->EventReceived.AddDynamic(
		this,
		&UGA_TwinSword_Primary::HandleComboWindow
	);

	ComboWindowTask->ReadyForActivation();
}

void UGA_TwinSword_Primary::BufferPrimaryInput()
{
	bHasBufferedPrimaryInput = true;
}

void UGA_TwinSword_Primary::TryConsumeBufferedInput()
{
	if (!bHasBufferedPrimaryInput) return;
	if (!bCanConsumeBufferedPrimaryInput) return;

	bHasBufferedPrimaryInput = false;
	bCanConsumeBufferedPrimaryInput = false;
	JumpToNextComboSection();
}

void UGA_TwinSword_Primary::JumpToNextComboSection()
{
	UAnimInstance* AnimInstance = CurrentActorInfo
		? CurrentActorInfo->GetAnimInstance()
		: nullptr;

	if (!AnimInstance || !ActiveMontage) return;

	const int32 CurrentSectionIndex = GetCurrentComboSectionIndex();
	if (CurrentSectionIndex == INDEX_NONE) return;

	const int32 NextSectionIndex = CurrentSectionIndex + 1;
	if (!ActiveComboSections.IsValidIndex(NextSectionIndex)) return;

	const FName NextSection = ActiveComboSections[NextSectionIndex];
	AnimInstance->Montage_JumpToSection(NextSection, ActiveMontage);
}

int32 UGA_TwinSword_Primary::GetCurrentComboSectionIndex() const
{
	UAnimInstance* AnimInstance = CurrentActorInfo
		? CurrentActorInfo->GetAnimInstance()
		: nullptr;

	if (!AnimInstance || !ActiveMontage) return INDEX_NONE;

	const FName CurrentSection = AnimInstance->Montage_GetCurrentSection(ActiveMontage);
	return ActiveComboSections.IndexOfByKey(CurrentSection);
}

void UGA_TwinSword_Primary::HandleInputPressed(float TimeWaited)
{
	BufferPrimaryInput();
	TryConsumeBufferedInput();
	WaitForNextPrimaryInput();
}

void UGA_TwinSword_Primary::HandleComboWindow(FGameplayEventData Payload)
{
	bCanConsumeBufferedPrimaryInput = true;
	TryConsumeBufferedInput();
}

void UGA_TwinSword_Primary::HandleMontageCompleted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_TwinSword_Primary::HandleMontageCancelled()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UGA_TwinSword_Primary::HandleMontageInterrupted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}
