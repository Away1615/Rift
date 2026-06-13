// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "RiftEnemyAnimInstance.generated.h"

UCLASS()
class RIFT_API URiftEnemyAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
	UPROPERTY(BlueprintReadOnly, Category="Movement")
	float Speed = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category="Movement")
	bool bIsMoving = false;
};
