#include "AbilitySystem/Abilities/GA_EnemyShieldBlock.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "AbilitySystem/RiftGameplayTags.h"
#include "Character/EnemyCharacter.h"
#include "Data/Ability/EnemyShieldBlockAbilityConfig.h"

UGA_EnemyShieldBlock::UGA_EnemyShieldBlock()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;

	FGameplayTagContainer ShieldBlockAssetTags;
	ShieldBlockAssetTags.AddTag(RiftGameplayTags::Ability_Enemy_ShieldBlock);
	SetAssetTags(ShieldBlockAssetTags);

	ActivationOwnedTags.AddTag(RiftGameplayTags::State_Attacking);
	ActivationBlockedTags.AddTag(RiftGameplayTags::State_Dead);
	ActivationBlockedTags.AddTag(RiftGameplayTags::State_Staggered);
	ActivationBlockedTags.AddTag(RiftGameplayTags::State_Blocking);
	ActivationBlockedTags.AddTag(RiftGameplayTags::State_Attacking);
}

void UGA_EnemyShieldBlock::ActivateAbility(
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

	const UEnemyShieldBlockAbilityConfig* ShieldBlockConfig =
		Cast<UEnemyShieldBlockAbilityConfig>(GetCurrentSourceObject());
	if (!ShieldBlockConfig || !ShieldBlockConfig->ShieldBlockMontage)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("GA_EnemyShieldBlock rejected: SourceObject must be EnemyShieldBlockAbilityConfig with ShieldBlockMontage. Ability=%s SourceObject=%s"),
			*GetNameSafe(this),
			*GetNameSafe(GetCurrentSourceObject())
		);
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	EnemyCharacter->SetCurrentBlockingPoiseDamageMultiplier(ShieldBlockConfig->BlockingPoiseDamageMultiplier);

	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		NAME_None,
		ShieldBlockConfig->ShieldBlockMontage,
		1.0f,
		NAME_None,
		true
	);
	if (!MontageTask)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	MontageTask->OnCompleted.AddDynamic(this, &UGA_EnemyShieldBlock::HandleMontageCompleted);
	MontageTask->OnBlendOut.AddDynamic(this, &UGA_EnemyShieldBlock::HandleMontageBlendOut);
	MontageTask->OnInterrupted.AddDynamic(this, &UGA_EnemyShieldBlock::HandleMontageInterrupted);
	MontageTask->OnCancelled.AddDynamic(this, &UGA_EnemyShieldBlock::HandleMontageCancelled);
	MontageTask->ReadyForActivation();
}

void UGA_EnemyShieldBlock::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const bool bReplicateEndAbility,
	const bool bWasCancelled
)
{
	if (ActorInfo)
	{
		if (AEnemyCharacter* EnemyCharacter = Cast<AEnemyCharacter>(ActorInfo->AvatarActor.Get()))
		{
			EnemyCharacter->SetBlockingState(false);
			EnemyCharacter->SetCurrentBlockingPoiseDamageMultiplier(1.0f);
		}
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_EnemyShieldBlock::HandleMontageCompleted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_EnemyShieldBlock::HandleMontageBlendOut()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_EnemyShieldBlock::HandleMontageInterrupted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UGA_EnemyShieldBlock::HandleMontageCancelled()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}
