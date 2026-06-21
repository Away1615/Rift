#include "AbilitySystem/Abilities/GA_PlayerGuard.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/RiftGameplayTags.h"
#include "Animation/AnimInstance.h"
#include "Character/PlayerCharacter.h"
#include "Data/Ability/GuardAbilityConfig.h"

UGA_PlayerGuard::UGA_PlayerGuard()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	bReplicateInputDirectly = true;

	FGameplayTagContainer GuardAssetTags;
	GuardAssetTags.AddTag(RiftGameplayTags::Ability_Guard);
	GuardAssetTags.AddTag(RiftGameplayTags::InputTag_Guard);
	SetAssetTags(GuardAssetTags);

	ActivationOwnedTags.AddTag(RiftGameplayTags::Ability_Guard);

	ActivationBlockedTags.AddTag(RiftGameplayTags::Ability_Guard);
	ActivationBlockedTags.AddTag(RiftGameplayTags::State_Dead);
	ActivationBlockedTags.AddTag(RiftGameplayTags::State_Hit_Light);
	ActivationBlockedTags.AddTag(RiftGameplayTags::State_Hit_Heavy);
	ActivationBlockedTags.AddTag(RiftGameplayTags::State_Blocking);
}

void UGA_PlayerGuard::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData
)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	bGuardEnding = false;
	bIsFinishingGuard = false;
	ActiveGuardMontage = nullptr;
	ActiveGuardStartSection = NAME_None;
	ActiveGuardLoopSection = NAME_None;
	ActiveGuardEndSection = NAME_None;

	if (!ActorInfo)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(ActorInfo->AvatarActor.Get());
	UAbilitySystemComponent* AbilitySystemComponent = ActorInfo->AbilitySystemComponent.Get();
	if (!PlayerCharacter || !AbilitySystemComponent)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	const FGameplayAbilitySpec* AbilitySpec = AbilitySystemComponent->FindAbilitySpecFromHandle(Handle);
	const UGuardAbilityConfig* GuardConfig = AbilitySpec
		? Cast<UGuardAbilityConfig>(AbilitySpec->SourceObject.Get())
		: nullptr;
	if (!GuardConfig || !GuardConfig->GuardMontage)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("Guard Activate failed: missing GuardAbilityConfig or GuardMontage. Character=%s SourceObject=%s"),
			*GetNameSafe(PlayerCharacter),
			AbilitySpec ? *GetNameSafe(AbilitySpec->SourceObject.Get()) : TEXT("None")
		);
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	PlayerCharacter->ClearMovementInputCache();
	PlayerCharacter->StopAssistedFacing();
	PlayerCharacter->SetFacingMode(ERiftCharacterFacingMode::Movement);

	ActiveGuardMontage = GuardConfig->GuardMontage;
	ActiveGuardStartSection = GuardConfig->GuardStartSection;
	ActiveGuardLoopSection = GuardConfig->GuardLoopSection;
	ActiveGuardEndSection = GuardConfig->GuardEndSection;
	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		NAME_None,
		GuardConfig->GuardMontage,
		1.0f,
		ActiveGuardStartSection,
		true
	);
	if (!MontageTask)
	{
		SetBlockingTag(AbilitySystemComponent, false);
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	MontageTask->OnCompleted.AddDynamic(this, &UGA_PlayerGuard::HandleMontageCompleted);
	MontageTask->OnBlendOut.AddDynamic(this, &UGA_PlayerGuard::HandleMontageBlendOut);
	MontageTask->OnInterrupted.AddDynamic(this, &UGA_PlayerGuard::HandleMontageInterrupted);
	MontageTask->OnCancelled.AddDynamic(this, &UGA_PlayerGuard::HandleMontageCancelled);
	MontageTask->ReadyForActivation();
	ConfigureGuardMontageSections();
	SetBlockingTag(AbilitySystemComponent, true);
}

void UGA_PlayerGuard::InputReleased(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo
)
{
	Super::InputReleased(Handle, ActorInfo, ActivationInfo);

	if (bGuardEnding)
	{
		return;
	}

	UAbilitySystemComponent* AbilitySystemComponent = ActorInfo
		? ActorInfo->AbilitySystemComponent.Get()
		: nullptr;
	SetBlockingTag(AbilitySystemComponent, false);

	bGuardEnding = true;

	if (ActiveGuardEndSection == NAME_None)
	{
		FinishGuardAbility(false);
		return;
	}

	ConfigureGuardMontageEndSection();
	JumpToGuardSection(ActiveGuardEndSection);
}

void UGA_PlayerGuard::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const bool bReplicateEndAbility,
	const bool bWasCancelled
)
{
	UAbilitySystemComponent* AbilitySystemComponent = ActorInfo
		? ActorInfo->AbilitySystemComponent.Get()
		: nullptr;
	SetBlockingTag(AbilitySystemComponent, false);

	if (ActorInfo)
	{
		if (APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(ActorInfo->AvatarActor.Get()))
		{
			PlayerCharacter->ClearActionCancelableState();
		}
	}

	ActiveGuardMontage = nullptr;
	ActiveGuardStartSection = NAME_None;
	ActiveGuardLoopSection = NAME_None;
	ActiveGuardEndSection = NAME_None;
	bGuardEnding = false;

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_PlayerGuard::SetBlockingTag(UAbilitySystemComponent* AbilitySystemComponent, const bool bBlocking)
{
	if (!AbilitySystemComponent) return;

	const int32 NewCount = bBlocking ? 1 : 0;
	AbilitySystemComponent->SetLooseGameplayTagCount(RiftGameplayTags::State_Blocking, NewCount);
	if (AbilitySystemComponent->IsOwnerActorAuthoritative())
	{
		AbilitySystemComponent->SetReplicatedLooseGameplayTagCount(RiftGameplayTags::State_Blocking, NewCount);
	}
}

void UGA_PlayerGuard::ConfigureGuardMontageSections()
{
	UAnimMontage* GuardMontage = ActiveGuardMontage.Get();
	const FGameplayAbilityActorInfo* ActorInfo = CurrentActorInfo;
	UAnimInstance* AnimInstance = ActorInfo ? ActorInfo->GetAnimInstance() : nullptr;
	if (!GuardMontage || !AnimInstance)
	{
		return;
	}

	if (ActiveGuardStartSection != NAME_None && ActiveGuardLoopSection != NAME_None)
	{
		AnimInstance->Montage_SetNextSection(ActiveGuardStartSection, ActiveGuardLoopSection, GuardMontage);
	}

	if (ActiveGuardLoopSection != NAME_None)
	{
		AnimInstance->Montage_SetNextSection(ActiveGuardLoopSection, ActiveGuardLoopSection, GuardMontage);
	}
}

void UGA_PlayerGuard::ConfigureGuardMontageEndSection()
{
	UAnimMontage* GuardMontage = ActiveGuardMontage.Get();
	const FGameplayAbilityActorInfo* ActorInfo = CurrentActorInfo;
	UAnimInstance* AnimInstance = ActorInfo ? ActorInfo->GetAnimInstance() : nullptr;
	if (!GuardMontage || !AnimInstance || ActiveGuardEndSection == NAME_None)
	{
		return;
	}

	AnimInstance->Montage_SetNextSection(ActiveGuardEndSection, NAME_None, GuardMontage);
}

void UGA_PlayerGuard::JumpToGuardSection(const FName SectionName)
{
	if (SectionName == NAME_None || !CurrentActorInfo)
	{
		return;
	}

	if (CurrentActorInfo->IsNetAuthority())
	{
		if (UAbilitySystemComponent* AbilitySystemComponent = CurrentActorInfo->AbilitySystemComponent.Get())
		{
			AbilitySystemComponent->CurrentMontageJumpToSection(SectionName);
		}
		return;
	}

	UAnimMontage* GuardMontage = ActiveGuardMontage.Get();
	UAnimInstance* AnimInstance = CurrentActorInfo->GetAnimInstance();
	if (GuardMontage && AnimInstance)
	{
		AnimInstance->Montage_JumpToSection(SectionName, GuardMontage);
	}
}

void UGA_PlayerGuard::FinishGuardAbility(const bool bWasCancelled)
{
	if (bIsFinishingGuard)
	{
		return;
	}

	bIsFinishingGuard = true;
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, bWasCancelled);
}

void UGA_PlayerGuard::HandleMontageCompleted()
{
	FinishGuardAbility(false);
}

void UGA_PlayerGuard::HandleMontageBlendOut()
{
	FinishGuardAbility(false);
}

void UGA_PlayerGuard::HandleMontageInterrupted()
{
	FinishGuardAbility(true);
}

void UGA_PlayerGuard::HandleMontageCancelled()
{
	FinishGuardAbility(true);
}
