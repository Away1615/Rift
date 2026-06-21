// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Combat/RiftHitReactionTypes.h"
#include "RiftEnemyAnimInstance.generated.h"

class AEnemyCharacter;
class UEnemyCharacterConfig;

UCLASS()
class RIFT_API URiftEnemyAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
	UPROPERTY(BlueprintReadOnly, Category="Movement")
	float Speed = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category="Movement")
	bool bIsMoving = false;

	UPROPERTY(BlueprintReadOnly, Category="Character")
	TObjectPtr<AEnemyCharacter> EnemyCharacter;

	UPROPERTY(BlueprintReadOnly, Category="Animation")
	TObjectPtr<UEnemyCharacterConfig> EnemyConfig;

	UPROPERTY(BlueprintReadOnly, Category="State")
	bool bIsDead = false;

	UPROPERTY(BlueprintReadOnly, Category="State")
	bool bIsStaggered = false;

	UPROPERTY(BlueprintReadOnly, Category="Hit")
	ERiftHitReactDirection LastHitReactDirection = ERiftHitReactDirection::Front;
};
