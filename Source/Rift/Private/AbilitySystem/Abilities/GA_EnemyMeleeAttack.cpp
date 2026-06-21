#include "AbilitySystem/Abilities/GA_EnemyMeleeAttack.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "AbilitySystem/RiftGameplayTags.h"
#include "Character/EnemyCharacter.h"
#include "Data/Ability/EnemyMeleeAttackAbilityConfig.h"

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
	if (!MeleeConfig || !MeleeConfig->AttackMontage)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("GA_EnemyMeleeAttack rejected: SourceObject must be EnemyMeleeAttackAbilityConfig with AttackMontage. Ability=%s SourceObject=%s"),
			*GetNameSafe(this),
			*GetNameSafe(GetCurrentSourceObject())
		);
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		NAME_None,
		MeleeConfig->AttackMontage,
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
