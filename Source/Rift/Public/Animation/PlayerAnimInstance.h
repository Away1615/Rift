// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Combat/RiftHitReactionTypes.h"
#include "PlayerAnimInstance.generated.h"

class APlayerCharacter;
class UPlayerAnimationConfig;

/**
 *
 */
UCLASS()
class RIFT_API UPlayerAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float Speed = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category="Character")
	TObjectPtr<APlayerCharacter> PlayerCharacter;

	UPROPERTY(BlueprintReadOnly, Category="Animation")
	TObjectPtr<UPlayerAnimationConfig> AnimationConfig;

	UPROPERTY(BlueprintReadOnly, Category="Hit")
	ERiftHitReactDirection LastHitReactDirection = ERiftHitReactDirection::Front;
};
