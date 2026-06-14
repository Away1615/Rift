// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "Character/BaseCharacter.h"
#include "TimerManager.h"
#include "EnemyCharacter.generated.h"

class UEnemyCharacterConfig;
class UAbilitySystemComponent;
class URiftAbilitySystemComponent;
class URiftEnemyAttributeSet;
class APlayerCharacter;
class UStaticMeshComponent;

UENUM(BlueprintType)
enum class ERiftHitReactDirection : uint8
{
	Front UMETA(DisplayName="Front"),
	Back UMETA(DisplayName="Back"),
	Left UMETA(DisplayName="Left"),
	Right UMETA(DisplayName="Right")
};

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

	void HandlePoiseHit(bool bPoiseBroken, const FVector& InstigatorLocation);

	void BeginAttackHitWindow();
	void TickAttackHitWindow();
	void EndAttackHitWindow();

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_PlayHitReact(ERiftHitReactDirection Direction);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayStagger(ERiftHitReactDirection Direction);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_ExitStagger();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Config")
	TObjectPtr<UEnemyCharacterConfig> EnemyCharacterConfig;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="AbilitySystem")
	TObjectPtr<URiftAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="AbilitySystem")
	TObjectPtr<URiftEnemyAttributeSet> AttributeSet;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Mesh")
	TObjectPtr<UStaticMeshComponent> Head;

private:
	void ApplyAnimationConfig() const;
	void ApplyCommonAttributesFromConfig();
	void ApplyWeaponsFromConfig();
	void GrantAbilities();
	void TryMeleeAttack();
	void ApplyPerfectDodgeStagger(APlayerCharacter* Dodger);
	void EnterStagger(ERiftHitReactDirection Direction, float Duration);
	void ExitStagger();
	void RestorePoise();
	ERiftHitReactDirection CalculateHitReactDirection(const FVector& InstigatorLocation) const;
	static FName GetHitReactSectionName(ERiftHitReactDirection Direction);

	FTimerHandle AttackDriverTimerHandle;
	FTimerHandle StaggerTimerHandle;
	FTimerHandle PoiseRegenTimerHandle;
	float NextAttackTime = 0.0f;
	TSet<TObjectKey<AActor>> HitPlayersThisAttack;
	TSet<TObjectKey<AActor>> PerfectDodgersThisAttack;
};
