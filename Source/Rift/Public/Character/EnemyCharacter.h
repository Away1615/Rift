// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "Character/BaseCharacter.h"
#include "Combat/RiftHitReactionTypes.h"
#include "TimerManager.h"
#include "EnemyCharacter.generated.h"

class UEnemyCharacterConfig;
class UEnemyCombatConfig;
class UAbilitySystemComponent;
class URiftAbilitySystemComponent;
class URiftEnemyAttributeSet;
class APlayerCharacter;
class UStaticMeshComponent;
class UUserWidget;
class UWidgetComponent;

/**
 *
 */
UCLASS()
class RIFT_API AEnemyCharacter : public ABaseCharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AEnemyCharacter();

	virtual void BeginPlay() override;
	virtual void PostInitializeComponents() override;
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UEnemyCharacterConfig* GetEnemyCharacterConfig() const { return EnemyCharacterConfig; }
	const UEnemyCombatConfig* GetEnemyCombatConfig() const;

	UFUNCTION(BlueprintPure, Category="Animation")
	bool IsStaggeredForAnimation() const;

	UFUNCTION(BlueprintPure, Category="Animation")
	bool IsDeadForAnimation() const;

	UFUNCTION(BlueprintPure, Category="Animation")
	ERiftHitReactDirection GetLastHitReactDirection() const { return LastHitReactDirection; }

	void HandlePoiseHit(bool bPoiseBroken, const FVector& InstigatorLocation);
	void HandleDeath(AActor* Killer);
	bool IsDeadForAI() const { return bIsDead; }
	bool IsStaggeredForAI() const;
	bool IsAttackingForAI() const;
	bool CanStartMeleeAttack(AActor* TargetActor) const;
	bool TryStartMeleeAttack(AActor* TargetActor);

	void BeginAttackHitWindow();
	void TickAttackHitWindow();
	void EndAttackHitWindow();

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_PlayHit(ERiftHitReactDirection Direction);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayDeath(ERiftHitReactDirection Direction);

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Config")
	TObjectPtr<UEnemyCharacterConfig> EnemyCharacterConfig;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="AbilitySystem")
	TObjectPtr<URiftAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="AbilitySystem")
	TObjectPtr<URiftEnemyAttributeSet> AttributeSet;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Mesh")
	TObjectPtr<UStaticMeshComponent> Head;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UWidgetComponent> HealthBarWidgetComp;

	UPROPERTY(EditDefaultsOnly, Category="UI")
	TSubclassOf<UUserWidget> HealthBarWidgetClass;

private:
	void ApplyAnimationConfig() const;
	void ApplyCommonAttributesFromConfig();
	void ApplyWeaponsFromConfig();
	void GrantAbilities();
	void StopAIMovement();
	void SetStaggeredState(bool bInStaggered);
	void EnterStaggered(float Duration);
	void ExitStaggered();
	void GrantKillReward(AActor* Killer);
	void FinishDeath();
	void RestorePoise();

	FTimerHandle StaggeredTimerHandle;
	FTimerHandle DeathDespawnTimerHandle;
	FTimerHandle PoiseRegenTimerHandle;
	float NextAttackTime = 0.0f;
	bool bIsDead = false;
	ERiftHitReactDirection LastHitReactDirection = ERiftHitReactDirection::Front;
	TSet<TObjectKey<AActor>> HitPlayersThisAttack;
};
