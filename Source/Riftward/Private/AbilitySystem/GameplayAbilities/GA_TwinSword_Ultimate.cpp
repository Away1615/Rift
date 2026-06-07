#include "AbilitySystem/GameplayAbilities/GA_TwinSword_Ultimate.h"

#include "Data/Player/Ability/AbilityDefinitionConfig.h"
#include "Data/Player/Ability/Fragments/AbilityMontageSectionsFragment.h"
#include "Data/Player/Ability/Fragments/AbilityTimedStateFragment.h"
#include "Data/Player/Input/AbilityInputID.h"

UGA_TwinSword_Ultimate::UGA_TwinSword_Ultimate()
{
	AbilityInputID = EAbilityInputID::Ultimate;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

void UGA_TwinSword_Ultimate::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	const UAbilityDefinitionConfig* AbilityDefinition = GetAbilityDefinition();
	const UAbilityMontageSectionsFragment* SectionsFragment = AbilityDefinition
		? AbilityDefinition->FindFragment<UAbilityMontageSectionsFragment>()
		: nullptr;

	if (!SectionsFragment || SectionsFragment->Sections.IsEmpty())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 施法期间应用阻塞状态（DataAsset Active State Tag 字段）
	const FGameplayTag ActiveStateTag = GetAbilityActiveStateTag();
	if (ActiveStateTag.IsValid())
	{
		CastingStateHandle = ApplyInfiniteStateTagEffect(Handle, ActorInfo, ActivationInfo, ActiveStateTag);
	}

	// 播放大招动画，动画完成后回调 OnAbilityMontageCompleted
	const FAbilityMontageSection& Section = SectionsFragment->Sections[0];
	UAnimMontage* MontageToPlay = SectionsFragment->Montage
		? SectionsFragment->Montage.Get()
		: Section.Montage.Get();

	if (!MontageToPlay || !TryPlayMontage(Handle, ActorInfo, MontageToPlay, Section.PlayRate, Section.SectionName, true))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
	}
}

bool UGA_TwinSword_Ultimate::OnAbilityMontageCompleted()
{
	// 移除施法阻塞 Tag
	RemoveGrantedStateTagEffect(CastingStateHandle);

	// 应用持续增强状态（从 TimedStateFragment 读取 Tag 和时长）
	const UAbilityDefinitionConfig* AbilityDefinition = GetAbilityDefinition();
	const UAbilityTimedStateFragment* TimedFrag = AbilityDefinition
		? AbilityDefinition->FindFragment<UAbilityTimedStateFragment>()
		: nullptr;

	if (TimedFrag && TimedFrag->ActiveStateTag.IsValid() && TimedFrag->Duration > 0.0f)
	{
		ApplyDurationStateTagEffect(
			CurrentSpecHandle,
			CurrentActorInfo,
			CurrentActivationInfo,
			TimedFrag->ActiveStateTag,
			TimedFrag->Duration
		);
	}

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	return false; // 告知基类不要再调用 EndAbility
}

void UGA_TwinSword_Ultimate::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const bool bReplicateEndAbility,
	const bool bWasCancelled)
{
	// 确保施法 Tag 被移除（中断/取消场景）
	RemoveGrantedStateTagEffect(CastingStateHandle);

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
