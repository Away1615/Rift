#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/GameplayAbilities/GA_PlayMontage.h"
#include "GameplayTagContainer.h"
#include "Data/Player/Ability/Fragments/AbilityActionExecutionFragment.h"
#include "GA_TwinSword_ConfigDrivenAction.generated.h"

UCLASS()
class RIFTWARD_API UGA_TwinSword_ConfigDrivenAction : public UMontageGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_TwinSword_ConfigDrivenAction();

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

private:
	EAbilityActionExecutionMode PendingExecutionMode = EAbilityActionExecutionMode::ForwardHit;

	// 施法中状态 Tag 句柄（用于阻止 Dodge 在释放期间打断本技能，参考 GA_TwinSword_Ultimate::CastingStateHandle）
	FActiveGameplayEffectHandle CastingStateHandle;

	bool PlayActionMontage(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo);

	// 执行实际效果（ForwardHit 或 ApplyTimedState）
	void ExecuteAction();

	bool ApplyTimedStateEffect() const;
	void ApplyForwardDamage() const;
	void ApplyDamageToTarget(AActor* TargetActor, const FHitResult& Hit, float Damage) const;

	UFUNCTION()
	void HandleExecutionEvent(FGameplayEventData Payload);
};
