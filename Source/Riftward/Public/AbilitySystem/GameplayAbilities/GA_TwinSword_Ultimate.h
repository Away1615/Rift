#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/GameplayAbilities/GA_PlayMontage.h"
#include "GameplayEffectTypes.h"
#include "GA_TwinSword_Ultimate.generated.h"

UCLASS()
class RIFTWARD_API UGA_TwinSword_Ultimate : public UMontageGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_TwinSword_Ultimate();

protected:
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled) override;

	// 动画播完后：移除施法阻塞 Tag，应用持续增强状态
	virtual bool OnAbilityMontageCompleted() override;

private:
	// 持有施法期间的无限 Tag GE（动画结束后移除）
	FActiveGameplayEffectHandle CastingStateHandle;
};
