// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Abilities/GA_ComboAttack.h"

#include "AbilitySystem/RiftGameplayTags.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Character/PlayerCharacter.h"
#include "Data/Player/PlayerClassConfig.h"
#include "Data/Player/Combat/PlayerCombatConfig.h"

UGA_ComboAttack::UGA_ComboAttack()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	FGameplayTagContainer ComboAttackAssetTags;
	ComboAttackAssetTags.AddTag(RiftGameplayTags::Ability_Attack_Primary);
	ComboAttackAssetTags.AddTag(RiftGameplayTags::InputTag_Attack_Primary);
	SetAssetTags(ComboAttackAssetTags);

	ActivationOwnedTags.AddTag(RiftGameplayTags::State_Attacking);
	ActivationBlockedTags.AddTag(RiftGameplayTags::State_Attacking);
}

void UGA_ComboAttack::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData
)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	bIsFinishingAttack = false;

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!ActorInfo)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(ActorInfo->AvatarActor.Get());
	if (!PlayerCharacter)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	const UPlayerClassConfig* PlayerClassConfig = PlayerCharacter->GetPlayerClassConfig();
	if (!PlayerClassConfig || !PlayerClassConfig->PlayerCombatConfig)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UAnimMontage* PrimaryAttackMontage = PlayerClassConfig->PlayerCombatConfig->PrimaryAttackMontage;
	if (!PrimaryAttackMontage)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	PlayerCharacter->SetFacingMode(ERiftCharacterFacingMode::CombatAssist);

	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		NAME_None,
		PrimaryAttackMontage,
		1.0f,
		NAME_None,
		true
	);

	if (!MontageTask)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	MontageTask->OnCompleted.AddDynamic(this, &UGA_ComboAttack::HandleMontageCompleted);
	MontageTask->OnBlendOut.AddDynamic(this, &UGA_ComboAttack::HandleMontageBlendOut);
	MontageTask->OnInterrupted.AddDynamic(this, &UGA_ComboAttack::HandleMontageInterrupted);
	MontageTask->OnCancelled.AddDynamic(this, &UGA_ComboAttack::HandleMontageCancelled);
	MontageTask->ReadyForActivation();
}

void UGA_ComboAttack::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled
)
{
	if (ActorInfo)
	{
		if (APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(ActorInfo->AvatarActor.Get()))
		{
			PlayerCharacter->StopAssistedFacing();
			PlayerCharacter->SetFacingMode(ERiftCharacterFacingMode::Movement);
		}
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_ComboAttack::HandleMontageCompleted()
{
	FinishAttackAbility(false);
}

void UGA_ComboAttack::HandleMontageBlendOut()
{
	FinishAttackAbility(false);
}

void UGA_ComboAttack::HandleMontageInterrupted()
{
	FinishAttackAbility(true);
}

void UGA_ComboAttack::HandleMontageCancelled()
{
	FinishAttackAbility(true);
}

void UGA_ComboAttack::FinishAttackAbility(const bool bWasCancelled)
{
	if (bIsFinishingAttack) return;

	bIsFinishingAttack = true;
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, bWasCancelled);
}
