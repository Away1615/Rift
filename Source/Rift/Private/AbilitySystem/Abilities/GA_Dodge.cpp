// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Abilities/GA_Dodge.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystem/Attributes/RiftPlayerAttributeSet.h"
#include "AbilitySystem/Effects/GE_StaminaCost.h"
#include "AbilitySystem/RiftGameplayTags.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Character/PlayerCharacter.h"
#include "Data/Player/PlayerClassConfig.h"
#include "Data/Player/Combat/PlayerCombatConfig.h"
#include "GameFramework/CharacterMovementComponent.h"

UGA_Dodge::UGA_Dodge()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	FGameplayTagContainer DodgeAssetTags;
	DodgeAssetTags.AddTag(RiftGameplayTags::Ability_Dodge);
	DodgeAssetTags.AddTag(RiftGameplayTags::InputTag_Core);
	SetAssetTags(DodgeAssetTags);

	ActivationOwnedTags.AddTag(RiftGameplayTags::Ability_Dodge);
	ActivationOwnedTags.AddTag(RiftGameplayTags::State_Dodging);
	ActivationBlockedTags.AddTag(RiftGameplayTags::Ability_Dodge);
}

void UGA_Dodge::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData
)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	bIsFinishingDodge = false;

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

	const UPlayerClassConfig* PlayerClassConfig = PlayerCharacter->GetPlayerClassConfig();
	const UPlayerCombatConfig* CombatConfig = PlayerClassConfig ? PlayerClassConfig->PlayerCombatConfig : nullptr;
	if (!CombatConfig || !CombatConfig->DodgeMontage)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	const float CurrentStamina = AbilitySystemComponent->GetNumericAttribute(
		URiftPlayerAttributeSet::GetStaminaAttribute()
	);
	if (CurrentStamina < CombatConfig->DodgeStaminaCost)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	FGameplayTagContainer AttackComboTags;
	AttackComboTags.AddTag(RiftGameplayTags::Ability_Attack_Combo);
	AbilitySystemComponent->CancelAbilities(&AttackComboTags);

	FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
	EffectContext.AddSourceObject(PlayerCharacter);

	FGameplayEffectSpecHandle DodgeCostSpec = AbilitySystemComponent->MakeOutgoingSpec(
		UGE_StaminaCost::StaticClass(),
		1.0f,
		EffectContext
	);
	if (!DodgeCostSpec.IsValid())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	DodgeCostSpec.Data->SetSetByCallerMagnitude(
		RiftGameplayTags::SetByCaller_StaminaCost,
		-CombatConfig->DodgeStaminaCost
	);
	AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*DodgeCostSpec.Data.Get());

	FVector DodgeDirection = PlayerCharacter->GetCameraRelativeMoveDirection();
	if (DodgeDirection.IsNearlyZero())
	{
		DodgeDirection = PlayerCharacter->GetActorForwardVector();
	}
	DodgeDirection.Z = 0.0f;
	DodgeDirection.Normalize();

	PlayerCharacter->SetActorRotation(DodgeDirection.ToOrientationRotator());

	UCharacterMovementComponent* MovementComponent = PlayerCharacter->GetCharacterMovement();
	if (!MovementComponent)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	bSavedOrientToMovement = MovementComponent->bOrientRotationToMovement;
	MovementComponent->bOrientRotationToMovement = false;

	PlayerCharacter->ActivatePerfectDodgeWindow(
		PlayerCharacter->GetActorLocation(),
		CombatConfig->PerfectDodgeWindowDuration
	);

	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		NAME_None,
		CombatConfig->DodgeMontage,
		1.0f,
		NAME_None,
		true
	);
	if (!MontageTask)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	MontageTask->OnCompleted.AddDynamic(this, &UGA_Dodge::HandleMontageCompleted);
	MontageTask->OnBlendOut.AddDynamic(this, &UGA_Dodge::HandleMontageBlendOut);
	MontageTask->OnInterrupted.AddDynamic(this, &UGA_Dodge::HandleMontageInterrupted);
	MontageTask->OnCancelled.AddDynamic(this, &UGA_Dodge::HandleMontageCancelled);
	MontageTask->ReadyForActivation();
}

void UGA_Dodge::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled
)
{
	RestoreOrientToMovement();

	if (ActorInfo)
	{
		if (APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(ActorInfo->AvatarActor.Get()))
		{
			if (UCharacterMovementComponent* Movement = PlayerCharacter->GetCharacterMovement())
			{
				Movement->StopMovementImmediately();
			}
		}
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_Dodge::HandleMontageCompleted()
{
	FinishDodgeAbility(false);
}

void UGA_Dodge::HandleMontageBlendOut()
{
	FinishDodgeAbility(false);
}

void UGA_Dodge::HandleMontageInterrupted()
{
	FinishDodgeAbility(true);
}

void UGA_Dodge::HandleMontageCancelled()
{
	FinishDodgeAbility(true);
}

void UGA_Dodge::FinishDodgeAbility(const bool bWasCancelled)
{
	if (bIsFinishingDodge) return;

	bIsFinishingDodge = true;
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, bWasCancelled);
}

void UGA_Dodge::RestoreOrientToMovement()
{
	if (!CurrentActorInfo) return;

	APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(CurrentActorInfo->AvatarActor.Get());
	if (!PlayerCharacter) return;

	UCharacterMovementComponent* MovementComponent = PlayerCharacter->GetCharacterMovement();
	if (!MovementComponent) return;

	MovementComponent->bOrientRotationToMovement = bSavedOrientToMovement;
}
