#include "AbilitySystem/Abilities/GA_TwinSwordSwordWave.h"

#include "AbilitySystem/RiftGameplayTags.h"

UGA_TwinSwordSwordWave::UGA_TwinSwordSwordWave()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	bReplicateInputDirectly = true;

	FGameplayTagContainer SwordWaveAssetTags;
	SwordWaveAssetTags.AddTag(RiftGameplayTags::Ability_Skill_TwinSword_SwordWave);
	SwordWaveAssetTags.AddTag(RiftGameplayTags::InputTag_Skill_E);
	SetAssetTags(SwordWaveAssetTags);

	ActivationOwnedTags.AddTag(RiftGameplayTags::State_Attacking);

	ActivationBlockedTags.AddTag(RiftGameplayTags::State_Dead);
	ActivationBlockedTags.AddTag(RiftGameplayTags::State_Hit_Light);
	ActivationBlockedTags.AddTag(RiftGameplayTags::State_Hit_Heavy);
}

void UGA_TwinSwordSwordWave::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData
)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	UE_LOG(LogTemp, Warning, TEXT("TwinSwordSwordWave is frozen and excluded from the current vertical slice."));
	EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
}

void UGA_TwinSwordSwordWave::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const bool bReplicateEndAbility,
	const bool bWasCancelled
)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_TwinSwordSwordWave::SpawnSwordWaveProjectile()
{
	UE_LOG(LogTemp, Warning, TEXT("TwinSwordSwordWave projectile spawn skipped because SwordWave is frozen."));
}

void UGA_TwinSwordSwordWave::HandleMontageCompleted()
{
	FinishSwordWaveAbility(false);
}

void UGA_TwinSwordSwordWave::HandleMontageBlendOut()
{
	FinishSwordWaveAbility(false);
}

void UGA_TwinSwordSwordWave::HandleMontageInterrupted()
{
	FinishSwordWaveAbility(true);
}

void UGA_TwinSwordSwordWave::HandleMontageCancelled()
{
	FinishSwordWaveAbility(true);
}

void UGA_TwinSwordSwordWave::FinishSwordWaveAbility(const bool bWasCancelled)
{
	if (bIsFinishingSwordWave) return;

	bIsFinishingSwordWave = true;
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, bWasCancelled);
}

void UGA_TwinSwordSwordWave::HandleRechargeTimer()
{
}
