// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/PlayerAnimInstance.h"

void UPlayerAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
}

void UPlayerAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	const APawn* Pawn = TryGetPawnOwner();
	if (!Pawn) return;

	Speed = Pawn->GetVelocity().Size2D();
}
