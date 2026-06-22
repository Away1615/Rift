// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include <cfloat>
#include "AbilitySystemInterface.h"
#include "AbilitySystem/RiftAttributeReactionReceiver.h"
#include "Character/BaseCharacter.h"
#include "Combat/RiftHitReactionTypes.h"
#include "Data/Ability/EnemyMeleeAttackAbilityConfig.h"
#include "TimerManager.h"
#include "EnemyCharacter.generated.h"

class UEnemyCharacterConfig;
class UEnemyShieldBlockAbilityConfig;
class UAbilitySystemComponent;
class UAnimMontage;
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
class RIFT_API AEnemyCharacter : public ABaseCharacter, public IAbilitySystemInterface, public IRiftAttributeReactionReceiver
{
	GENERATED_BODY()

public:
	AEnemyCharacter();

	virtual void BeginPlay() override;
	virtual void PostInitializeComponents() override;
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UEnemyCharacterConfig* GetEnemyCharacterConfig() const { return EnemyCharacterConfig; }
	void SetEnemyCharacterConfig(UEnemyCharacterConfig* NewEnemyCharacterConfig);
	const UEnemyMeleeAttackAbilityConfig* GetEnemyMeleeAttackAbilityConfig() const;
	const UEnemyShieldBlockAbilityConfig* GetEnemyShieldBlockAbilityConfig() const;

	UFUNCTION(BlueprintCallable, Category="AI")
	void StartEnemyIntroOrAI();

	UFUNCTION(BlueprintPure, Category="Animation")
	bool IsStaggeredForAnimation() const;

	UFUNCTION(BlueprintPure, Category="Animation")
	bool IsDeadForAnimation() const;

	UFUNCTION(BlueprintPure, Category="Combat|Blocking")
	bool IsBlocking() const;

	UFUNCTION(BlueprintPure, Category="Combat|SuperArmor")
	bool IsEnemySuperArmor() const;

	UFUNCTION(BlueprintPure, Category="Animation")
	ERiftHitReactDirection GetLastHitReactDirection() const { return LastHitReactDirection; }

	void HandlePoiseHit(bool bPoiseBroken, const FVector& InstigatorLocation);
	void HandleDeath(AActor* Killer);
	virtual void HandleAttributeDeath(AActor* DeathInstigator) override;
	virtual void HandleAttributePoiseHit(bool bPoiseBroken, AActor* DamageInstigator) override;
	virtual void HandleAttributeDamageNumber(float DamageAmount, bool bBlocked, const FVector& WorldLocation) override;
	virtual float GetAttributeBlockingPoiseDamageMultiplier() const override;
	void SetCurrentBlockingPoiseDamageMultiplier(float NewMultiplier);
	void SetBlockingState(bool bInBlocking);
	void SetEnemySuperArmorState(bool bInSuperArmor);
	bool IsDeadForAI() const { return bIsDead; }
	bool IsStaggeredForAI() const;
	bool IsAttackingForAI() const;
	bool IsIntroForAI() const;
	bool IsRetreatingForAI() const;
	bool IsBusyForAI() const;
	UFUNCTION(BlueprintPure, Category="Combat|Phase")
	virtual int32 GetCurrentCombatPhase() const;
	virtual bool IsCombatPhaseTransitioningForAI() const;
	bool CanStartMeleeAttack(AActor* TargetActor) const;
	bool TryStartMeleeAttack(AActor* TargetActor);
	bool CanStartShieldBlock(AActor* TargetActor) const;
	bool TryStartShieldBlock(AActor* TargetActor);
	const FRiftEnemyMeleeAttackVariant* GetCurrentMeleeAttackVariant() const;
	AActor* GetCurrentMeleeAttackTarget() const;

	void BeginAttackHitWindow();
	void TickAttackHitWindow();
	void EndAttackHitWindow();

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_PlayHit(ERiftHitReactDirection Direction);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayDeath(ERiftHitReactDirection Direction);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlaySpawnIntro();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayStaggered();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayHitRetreat();

	UFUNCTION(BlueprintImplementableEvent, Category="Combat|DamageNumber")
	void OnDamageNumber(float DamageAmount, bool bBlocked, FVector WorldLocation);

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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="AI")
	bool bAutoStartSpawnIntroOrAI = true;

private:
	void ApplyPresentationConfig() const;
	void ApplyCommonAttributesFromConfig();
	void ApplyWeaponsFromConfig();
	void GrantAbilities();
	void StopAIMovement();
	void StartEnemyBehavior();
	void StartSpawnIntroOrAI();
	void FinishSpawnIntro();
	void PlaySpawnIntroMontage();
	void OnSpawnIntroMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	void SetStaggeredState(bool bInStaggered);
	void SetIntroState(bool bInIntro);
	void SetHitRetreatState(bool bInRetreating);
	void EnterStaggered(float Duration);
	void ExitStaggered();
	void OnStaggeredMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	void GrantKillReward(AActor* Killer);
	void FinishDeath();
	void RestorePoise();
	bool TrySelectMeleeAttackVariant(AActor* TargetActor, FRiftEnemyMeleeAttackVariant& OutVariant) const;
	bool CanStartHitRetreat() const;
	bool TryStartHitRetreat(const FVector& InstigatorLocation, bool bWasBlocking);
	void OnHitRetreatMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	FTimerHandle StaggeredTimerHandle;
	FTimerHandle SpawnIntroTimerHandle;
	FTimerHandle DeathDespawnTimerHandle;
	FTimerHandle PoiseRegenTimerHandle;
	float NextAttackTime = 0.0f;
	float LastShieldBlockTime = -FLT_MAX;
	float LastHitRetreatTime = -FLT_MAX;
	float CurrentBlockingPoiseDamageMultiplier = 1.0f;
	bool bHasStartedEnemyBehavior = false;
	bool bIsDead = false;
	bool bIsHitRetreating = false;
	bool bHasCurrentMeleeAttackVariant = false;
	ERiftHitReactDirection LastHitReactDirection = ERiftHitReactDirection::Front;
	TWeakObjectPtr<UAnimMontage> ActiveSpawnIntroMontage;
	TWeakObjectPtr<UAnimMontage> ActiveStaggeredMontage;
	TWeakObjectPtr<UAnimMontage> ActiveHitRetreatMontage;
	TWeakObjectPtr<AActor> CurrentMeleeAttackTarget;
	TSet<TObjectKey<AActor>> HitPlayersThisAttack;
	FRiftEnemyMeleeAttackVariant CurrentMeleeAttackVariant;
};
