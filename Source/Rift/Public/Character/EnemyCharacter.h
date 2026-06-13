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

	void HandlePoiseHit(bool bPoiseBroken, const FVector& InstigatorLocation);

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_PlayHitReact(ERiftHitReactDirection Direction, bool bPoiseBroken);

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Config")
	TObjectPtr<UEnemyCharacterConfig> EnemyCharacterConfig;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="AbilitySystem")
	TObjectPtr<URiftAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="AbilitySystem")
	TObjectPtr<URiftEnemyAttributeSet> AttributeSet;

private:
	void ApplyAnimationConfig() const;
	void RestorePoise();
	ERiftHitReactDirection CalculateHitReactDirection(const FVector& InstigatorLocation) const;
	static FName GetHitReactSectionName(ERiftHitReactDirection Direction);

	FTimerHandle PoiseRegenTimerHandle;
};
