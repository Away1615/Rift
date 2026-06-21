#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_EnemyMeleeAttack.generated.h"

class AEnemyCharacter;
class UCharacterMovementComponent;
class UMotionWarpingComponent;

UCLASS()
class RIFT_API UGA_EnemyMeleeAttack : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_EnemyMeleeAttack();

	virtual void ActivateAbility(
		FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData
	) override;

protected:
	UFUNCTION()
	void HandleMontageCompleted();

	UFUNCTION()
	void HandleMontageBlendOut();

	UFUNCTION()
	void HandleMontageInterrupted();

	UFUNCTION()
	void HandleMontageCancelled();

	virtual void EndAbility(
		FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled
	) override;

private:
	void ApplyRootMotionAttackMovementLock(AEnemyCharacter* EnemyCharacter);
	void RestoreRootMotionAttackMovementLock();
	void ApplyAttackMotionWarping(AEnemyCharacter* EnemyCharacter);
	void ClearAttackMotionWarping();

	TWeakObjectPtr<UCharacterMovementComponent> CachedMovementComponent;
	TWeakObjectPtr<UMotionWarpingComponent> CachedMotionWarpingComponent;
	FName ActiveMotionWarpTargetName = NAME_None;
	bool bSavedOrientRotationToMovement = false;
	bool bHasSavedOrientRotationToMovement = false;
};
