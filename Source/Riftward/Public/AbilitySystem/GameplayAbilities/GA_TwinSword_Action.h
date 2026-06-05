#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/GameplayAbilities/BaseGameplayAbility.h"
#include "GameplayTagContainer.h"
#include "GA_TwinSword_Action.generated.h"

UCLASS()
class RIFTWARD_API UGA_TwinSword_Action : public UBaseGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_TwinSword_Action();

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
	FGameplayTag ActiveBuffTag;
	bool bAddedBuffTag = false;

	void ActivateSignature(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo);

	void ActivateEnhance(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo);

	void PlayConfiguredMontage(bool bEndOnMontageFinished);
	void ApplyForwardDamage() const;
	void ApplyDamageToTarget(AActor* TargetActor, const FHitResult& Hit) const;

	UFUNCTION()
	void HandleBuffFinished();

	UFUNCTION()
	void HandleMontageCompleted();

	UFUNCTION()
	void HandleMontageCancelled();

	UFUNCTION()
	void HandleMontageInterrupted();
};
