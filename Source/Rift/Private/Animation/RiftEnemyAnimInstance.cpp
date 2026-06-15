// Fill out your copyright notice in the Description page of Project Settings.

#include "Animation/RiftEnemyAnimInstance.h"

#include "Character/EnemyCharacter.h"
#include "Data/Enemy/Animation/EnemyAnimationConfig.h"
#include "Data/Enemy/EnemyCharacterConfig.h"
#include "GameFramework/Pawn.h"

void URiftEnemyAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	APawn* Pawn = TryGetPawnOwner();
	if (!Pawn)
	{
		Speed = 0.0f;
		bIsMoving = false;
		EnemyCharacter = nullptr;
		AnimationConfig = nullptr;
		bIsDead = false;
		bIsStaggered = false;
		LastHitReactDirection = ERiftHitReactDirection::Front;
		StaggeredStartSequence = nullptr;
		StaggeredLoopSequence = nullptr;
		StaggeredEndSequence = nullptr;
		return;
	}

	Speed = Pawn->GetVelocity().Size2D();
	bIsMoving = Speed > UE_KINDA_SMALL_NUMBER;
	EnemyCharacter = Cast<AEnemyCharacter>(Pawn);
	if (!EnemyCharacter)
	{
		AnimationConfig = nullptr;
		bIsDead = false;
		bIsStaggered = false;
		LastHitReactDirection = ERiftHitReactDirection::Front;
		StaggeredStartSequence = nullptr;
		StaggeredLoopSequence = nullptr;
		StaggeredEndSequence = nullptr;
		return;
	}

	bIsDead = EnemyCharacter->IsDeadForAnimation();
	bIsStaggered = EnemyCharacter->IsStaggeredForAnimation();
	LastHitReactDirection = EnemyCharacter->GetLastHitReactDirection();

	const UEnemyCharacterConfig* EnemyCharacterConfig = EnemyCharacter->GetEnemyCharacterConfig();
	AnimationConfig = EnemyCharacterConfig ? EnemyCharacterConfig->EnemyAnimationConfig : nullptr;
	if (!AnimationConfig)
	{
		StaggeredStartSequence = nullptr;
		StaggeredLoopSequence = nullptr;
		StaggeredEndSequence = nullptr;
		return;
	}

	StaggeredStartSequence = AnimationConfig->StaggeredStartSequence;
	StaggeredLoopSequence = AnimationConfig->StaggeredLoopSequence;
	StaggeredEndSequence = AnimationConfig->StaggeredEndSequence;
}
