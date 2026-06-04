// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/GameplayAbilities/GA_TwinSword_Combo.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Abilities/Tasks/AbilityTask_WaitInputPress.h"
#include "Animation/AnimInstance.h"
#include "Character/PlayerCharacter.h"
#include "Data/Player/Ability/TwinSword/TwinSwordComboAbilityConfig.h"
#include "Debug/Logger.h"
#include "GameplayTags/RiftGameplayTags.h"

void UGA_TwinSword_Combo::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
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

	const UTwinSwordComboAbilityConfig* ComboConfig = GetComboConfig();
	if (!ComboConfig)
	{
		Logger::Error(PlayerCharacter, TEXT("TwinSword ComboAbilityConfig is missing"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo();
	const FRiftGameplayTags& RiftTags = FRiftGameplayTags::Get();
	const bool bUseSuperAttack = AbilitySystemComponent
		&& AbilitySystemComponent->HasMatchingGameplayTag(RiftTags.State_Ability_TwinSword_Core_PerfectDodgeEmpowered);

	UAnimMontage* MontageToPlay = bUseSuperAttack
		? ComboConfig->SuperAttackMontage
		: ComboConfig->AttackMontage;

	if (!MontageToPlay)
	{
		Logger::Error(PlayerCharacter, bUseSuperAttack
			? TEXT("TwinSword super attack montage is missing")
			: TEXT("TwinSword attack montage is missing"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (ComboConfig->ComboSections.IsEmpty())
	{
		Logger::Error(PlayerCharacter, TEXT("TwinSword combo sections are missing"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (ShouldCommitComboAbility() && !CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (bUseSuperAttack)
	{
		AbilitySystemComponent->RemoveLooseGameplayTag(RiftTags.State_Ability_TwinSword_Core_PerfectDodgeEmpowered);
	}

	AbilitySystemComponent->AddLooseGameplayTag(GetAbilityActiveStateTag());
	bAddedComboActiveTag = true;

	ActiveMontage = MontageToPlay;
	ActiveComboSections = ComboConfig->ComboSections;
	bHasBufferedInput = false;
	bCanConsumeBufferedInput = false;

	UAbilityTask_PlayMontageAndWait* MontageTask =
		UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this,
			NAME_None,
			ActiveMontage,
			1.0f,
			ActiveComboSections[0],
			true
		);

	MontageTask->OnCompleted.AddDynamic(this, &UGA_TwinSword_Combo::HandleMontageCompleted);
	MontageTask->OnCancelled.AddDynamic(this, &UGA_TwinSword_Combo::HandleMontageCancelled);
	MontageTask->OnInterrupted.AddDynamic(this, &UGA_TwinSword_Combo::HandleMontageInterrupted);
	MontageTask->ReadyForActivation();

	ResetComboSectionLinks();
	WaitForComboWindow();
	WaitForComboInput();
}

void UGA_TwinSword_Combo::EndAbility(const FGameplayAbilitySpecHandle Handle,
                                     const FGameplayAbilityActorInfo* ActorInfo,
                                     const FGameplayAbilityActivationInfo ActivationInfo,
                                     bool bReplicateEndAbility,
                                     bool bWasCancelled)
{
	if (bAddedComboActiveTag)
	{
		if (UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo())
		{
			AbilitySystemComponent->RemoveLooseGameplayTag(GetAbilityActiveStateTag());
		}
	}

	ActiveMontage = nullptr;
	ActiveComboSections.Empty();
	bHasBufferedInput = false;
	bCanConsumeBufferedInput = false;
	bAddedComboActiveTag = false;

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

FGameplayTag UGA_TwinSword_Combo::GetComboWindowEventTag() const
{
	return FGameplayTag();
}

FGameplayTag UGA_TwinSword_Combo::GetAbilityActiveStateTag() const
{
	return FGameplayTag();
}

bool UGA_TwinSword_Combo::ShouldCommitComboAbility() const
{
	return false;
}

const UTwinSwordComboAbilityConfig* UGA_TwinSword_Combo::GetComboConfig() const
{
	const FGameplayAbilitySpec* Spec = GetCurrentAbilitySpec();
	return Spec
		? Cast<UTwinSwordComboAbilityConfig>(Spec->SourceObject.Get())
		: nullptr;
}

void UGA_TwinSword_Combo::ResetComboSectionLinks() const
{
	UAnimInstance* AnimInstance = CurrentActorInfo
		? CurrentActorInfo->GetAnimInstance()
		: nullptr;

	if (!AnimInstance || !ActiveMontage) return;

	UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo();

	for (const FName ComboSection : ActiveComboSections)
	{
		if (CurrentActorInfo->IsNetAuthority() && AbilitySystemComponent)
		{
			AbilitySystemComponent->CurrentMontageSetNextSectionName(ComboSection, NAME_None);
		}
		else
		{
			AnimInstance->Montage_SetNextSection(ComboSection, NAME_None, ActiveMontage);
		}
	}
}

void UGA_TwinSword_Combo::WaitForComboWindow()
{
	const FGameplayTag ComboWindowEventTag = GetComboWindowEventTag();
	if (!ComboWindowEventTag.IsValid()) return;

	UAbilityTask_WaitGameplayEvent* ComboWindowTask =
		UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
			this,
			ComboWindowEventTag,
			nullptr,
			false,
			true
		);

	ComboWindowTask->EventReceived.AddDynamic(
		this,
		&UGA_TwinSword_Combo::HandleComboWindow
	);

	ComboWindowTask->ReadyForActivation();
}

void UGA_TwinSword_Combo::WaitForComboInput()
{
	UAbilityTask_WaitInputPress* ComboInputTask =
		UAbilityTask_WaitInputPress::WaitInputPress(this, false);

	ComboInputTask->OnPress.AddDynamic(
		this,
		&UGA_TwinSword_Combo::HandleComboInputPressed
	);

	ComboInputTask->ReadyForActivation();
}

void UGA_TwinSword_Combo::BufferInput()
{
	bHasBufferedInput = true;
}

void UGA_TwinSword_Combo::TryConsumeBufferedInput()
{
	if (!bHasBufferedInput) return;
	if (!bCanConsumeBufferedInput) return;

	bHasBufferedInput = false;
	bCanConsumeBufferedInput = false;
	JumpToNextComboSection();
}

void UGA_TwinSword_Combo::JumpToNextComboSection()
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

	if (CurrentActorInfo->IsNetAuthority())
	{
		if (UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo())
		{
			AbilitySystemComponent->CurrentMontageJumpToSection(NextSection);
		}
	}
	else if (CurrentActorInfo->IsLocallyControlled())
	{
		AnimInstance->Montage_JumpToSection(NextSection, ActiveMontage);
	}
}

int32 UGA_TwinSword_Combo::GetCurrentComboSectionIndex() const
{
	UAnimInstance* AnimInstance = CurrentActorInfo
		? CurrentActorInfo->GetAnimInstance()
		: nullptr;

	if (!AnimInstance || !ActiveMontage) return INDEX_NONE;

	const FName CurrentSection = AnimInstance->Montage_GetCurrentSection(ActiveMontage);
	return ActiveComboSections.IndexOfByKey(CurrentSection);
}

void UGA_TwinSword_Combo::HandleComboWindow(FGameplayEventData Payload)
{
	bCanConsumeBufferedInput = true;
	TryConsumeBufferedInput();
}

void UGA_TwinSword_Combo::HandleComboInputPressed(float TimeWaited)
{
	BufferInput();
	TryConsumeBufferedInput();
	WaitForComboInput();
}

void UGA_TwinSword_Combo::HandleMontageCompleted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_TwinSword_Combo::HandleMontageCancelled()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UGA_TwinSword_Combo::HandleMontageInterrupted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}
