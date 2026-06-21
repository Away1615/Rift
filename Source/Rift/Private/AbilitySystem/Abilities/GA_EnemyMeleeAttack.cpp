#include "AbilitySystem/Abilities/GA_EnemyMeleeAttack.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "AbilitySystem/RiftGameplayTags.h"
#include "Character/EnemyCharacter.h"
#include "Data/Ability/EnemyMeleeAttackAbilityConfig.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "MotionWarpingComponent.h"

UGA_EnemyMeleeAttack::UGA_EnemyMeleeAttack()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;

	FGameplayTagContainer MeleeAttackAssetTags;
	MeleeAttackAssetTags.AddTag(RiftGameplayTags::Ability_Enemy_MeleeAttack);
	SetAssetTags(MeleeAttackAssetTags);

	ActivationOwnedTags.AddTag(RiftGameplayTags::State_Attacking);
	ActivationBlockedTags.AddTag(RiftGameplayTags::State_Dead);
	ActivationBlockedTags.AddTag(RiftGameplayTags::State_Staggered);
	ActivationBlockedTags.AddTag(RiftGameplayTags::State_Blocking);
}

void UGA_EnemyMeleeAttack::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData
)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!ActorInfo)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	AEnemyCharacter* EnemyCharacter = Cast<AEnemyCharacter>(ActorInfo->AvatarActor.Get());
	if (!EnemyCharacter)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	const UEnemyMeleeAttackAbilityConfig* MeleeConfig =
		Cast<UEnemyMeleeAttackAbilityConfig>(GetCurrentSourceObject());
	if (!MeleeConfig)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("GA_EnemyMeleeAttack rejected: SourceObject must be EnemyMeleeAttackAbilityConfig. Ability=%s SourceObject=%s"),
			*GetNameSafe(this),
			*GetNameSafe(GetCurrentSourceObject())
		);
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	const FRiftEnemyMeleeAttackVariant* CurrentVariant = EnemyCharacter->GetCurrentMeleeAttackVariant();
	if (!CurrentVariant || !CurrentVariant->AttackMontage)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("GA_EnemyMeleeAttack rejected: no selected variant AttackMontage. Ability=%s SourceObject=%s"),
			*GetNameSafe(this),
			*GetNameSafe(GetCurrentSourceObject())
		);
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	ApplyRootMotionAttackMovementLock(EnemyCharacter);
	ApplyAttackMotionWarping(EnemyCharacter);

	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		NAME_None,
		CurrentVariant->AttackMontage,
		1.0f,
		NAME_None,
		true
	);
	if (!MontageTask)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	MontageTask->OnCompleted.AddDynamic(this, &UGA_EnemyMeleeAttack::HandleMontageCompleted);
	MontageTask->OnBlendOut.AddDynamic(this, &UGA_EnemyMeleeAttack::HandleMontageBlendOut);
	MontageTask->OnInterrupted.AddDynamic(this, &UGA_EnemyMeleeAttack::HandleMontageInterrupted);
	MontageTask->OnCancelled.AddDynamic(this, &UGA_EnemyMeleeAttack::HandleMontageCancelled);
	MontageTask->ReadyForActivation();
}

void UGA_EnemyMeleeAttack::HandleMontageCompleted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_EnemyMeleeAttack::HandleMontageBlendOut()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_EnemyMeleeAttack::HandleMontageInterrupted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UGA_EnemyMeleeAttack::HandleMontageCancelled()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UGA_EnemyMeleeAttack::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const bool bReplicateEndAbility,
	const bool bWasCancelled
)
{
	ClearAttackMotionWarping();
	RestoreRootMotionAttackMovementLock();

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_EnemyMeleeAttack::ApplyRootMotionAttackMovementLock(AEnemyCharacter* EnemyCharacter)
{
	if (!EnemyCharacter)
	{
		return;
	}

	UCharacterMovementComponent* Movement = EnemyCharacter->GetCharacterMovement();
	if (!Movement)
	{
		return;
	}

	CachedMovementComponent = Movement;
	bSavedOrientRotationToMovement = Movement->bOrientRotationToMovement;
	bHasSavedOrientRotationToMovement = true;

	Movement->bOrientRotationToMovement = false;
}

void UGA_EnemyMeleeAttack::RestoreRootMotionAttackMovementLock()
{
	if (!bHasSavedOrientRotationToMovement)
	{
		return;
	}

	if (UCharacterMovementComponent* Movement = CachedMovementComponent.Get())
	{
		Movement->bOrientRotationToMovement = bSavedOrientRotationToMovement;
	}

	CachedMovementComponent.Reset();
	bSavedOrientRotationToMovement = false;
	bHasSavedOrientRotationToMovement = false;
}

void UGA_EnemyMeleeAttack::ApplyAttackMotionWarping(AEnemyCharacter* EnemyCharacter)
{
	if (!EnemyCharacter)
	{
		return;
	}

	const FRiftEnemyMeleeAttackVariant* CurrentVariant = EnemyCharacter->GetCurrentMeleeAttackVariant();
	if (!CurrentVariant || !CurrentVariant->bUseMotionWarping || CurrentVariant->MotionWarpTargetName.IsNone())
	{
		return;
	}

	AActor* TargetActor = EnemyCharacter->GetCurrentMeleeAttackTarget();
	if (!TargetActor)
	{
		return;
	}

	UMotionWarpingComponent* MotionWarpingComponent = EnemyCharacter->FindComponentByClass<UMotionWarpingComponent>();
	if (!MotionWarpingComponent)
	{
		return;
	}

	FVector ToTarget = TargetActor->GetActorLocation() - EnemyCharacter->GetActorLocation();
	ToTarget.Z = 0.0f;
	if (!ToTarget.Normalize())
	{
		return;
	}

	const float StopDistance = FMath::Max(0.0f, CurrentVariant->MotionWarpStopDistance);
	const FVector WarpTargetLocation = TargetActor->GetActorLocation() - ToTarget * StopDistance;

	MotionWarpingComponent->AddOrUpdateWarpTargetFromLocation(
		CurrentVariant->MotionWarpTargetName,
		WarpTargetLocation
	);

	CachedMotionWarpingComponent = MotionWarpingComponent;
	ActiveMotionWarpTargetName = CurrentVariant->MotionWarpTargetName;
}

void UGA_EnemyMeleeAttack::ClearAttackMotionWarping()
{
	if (UMotionWarpingComponent* MotionWarpingComponent = CachedMotionWarpingComponent.Get())
	{
		if (!ActiveMotionWarpTargetName.IsNone())
		{
			MotionWarpingComponent->RemoveWarpTarget(ActiveMotionWarpTargetName);
		}
	}

	CachedMotionWarpingComponent.Reset();
	ActiveMotionWarpTargetName = NAME_None;
}
