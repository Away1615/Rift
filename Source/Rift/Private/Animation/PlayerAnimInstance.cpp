// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/PlayerAnimInstance.h"

#include "Character/PlayerCharacter.h"
#include "Data/Player/Animation/PlayerAnimationConfig.h"

void UPlayerAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	APawn* Pawn = TryGetPawnOwner();
	if (!Pawn)
	{
		Speed = 0.0f;
		PlayerCharacter = nullptr;
		AnimationConfig = nullptr;
		LastHitReactDirection = ERiftHitReactDirection::Front;
		return;
	}

	Speed = Pawn->GetVelocity().Size2D();
	PlayerCharacter = Cast<APlayerCharacter>(Pawn);
	if (!PlayerCharacter)
	{
		AnimationConfig = nullptr;
		LastHitReactDirection = ERiftHitReactDirection::Front;
		return;
	}

	LastHitReactDirection = PlayerCharacter->GetLastHitReactDirection();
	AnimationConfig = PlayerCharacter->GetPlayerAnimationConfig();
}
