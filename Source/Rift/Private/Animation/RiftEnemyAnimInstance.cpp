// Fill out your copyright notice in the Description page of Project Settings.

#include "Animation/RiftEnemyAnimInstance.h"

#include "GameFramework/Pawn.h"

void URiftEnemyAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	const APawn* Pawn = TryGetPawnOwner();
	if (!Pawn)
	{
		Speed = 0.0f;
		bIsMoving = false;
		return;
	}

	Speed = Pawn->GetVelocity().Size2D();
	bIsMoving = Speed > UE_KINDA_SMALL_NUMBER;
}
