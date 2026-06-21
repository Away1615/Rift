#include "AbilitySystem/Abilities/GA_TwinSwordSkillQ.h"

#include "AbilitySystem/RiftGameplayTags.h"

UGA_TwinSwordSkillQ::UGA_TwinSwordSkillQ()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	bReplicateInputDirectly = true;

	FGameplayTagContainer SkillQAssetTags;
	SkillQAssetTags.AddTag(RiftGameplayTags::Ability_Skill_TwinSword_Q);
	SkillQAssetTags.AddTag(RiftGameplayTags::InputTag_Skill_Q);
	SetAssetTags(SkillQAssetTags);

	ActivationOwnedTags.AddTag(RiftGameplayTags::State_Attacking);

	ActivationBlockedTags.AddTag(RiftGameplayTags::State_Attacking);
	ActivationBlockedTags.AddTag(RiftGameplayTags::State_Dead);
	ActivationBlockedTags.AddTag(RiftGameplayTags::State_Hit_Light);
	ActivationBlockedTags.AddTag(RiftGameplayTags::State_Hit_Heavy);
}

void UGA_TwinSwordSkillQ::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData
)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	UE_LOG(LogTemp, Warning, TEXT("TwinSword Q / blade skill is frozen and excluded from the current vertical slice."));
	EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
}

void UGA_TwinSwordSkillQ::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const bool bReplicateEndAbility,
	const bool bWasCancelled
)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_TwinSwordSkillQ::HandleMontageCompleted()
{
	FinishSkillQAbility(false);
}

void UGA_TwinSwordSkillQ::HandleMontageBlendOut()
{
	FinishSkillQAbility(false);
}

void UGA_TwinSwordSkillQ::HandleMontageInterrupted()
{
	FinishSkillQAbility(true);
}

void UGA_TwinSwordSkillQ::HandleMontageCancelled()
{
	FinishSkillQAbility(true);
}

void UGA_TwinSwordSkillQ::FinishSkillQAbility(const bool bWasCancelled)
{
	if (bIsFinishingSkillQ) return;

	bIsFinishingSkillQ = true;
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, bWasCancelled);
}
