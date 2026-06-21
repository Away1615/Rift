// Fill out your copyright notice in the Description page of Project Settings.

#include "Combat/RiftTargetAssistComponent.h"

#include "Character/EnemyCharacter.h"
#include "Character/PlayerCharacter.h"
#include "Debug/RiftDebugCVars.h"
#include "DrawDebugHelpers.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"

URiftTargetAssistComponent::URiftTargetAssistComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

AActor* URiftTargetAssistComponent::FindSoftTarget() const
{
	const APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(GetOwner());
	if (!PlayerCharacter) return nullptr;

	const float MaxRange = TargetAssistMaxRange;
	const float MaxAngleDegrees = TargetAssistMaxAngleDegrees;
	if (MaxRange <= 0.0f || MaxAngleDegrees <= 0.0f) return nullptr;

	UWorld* World = GetWorld();
	if (!World) return nullptr;

	FVector ReferenceDirection = FVector::ZeroVector;
	if (const UCharacterMovementComponent* MovementComponent = PlayerCharacter->GetCharacterMovement())
	{
		ReferenceDirection = MovementComponent->GetCurrentAcceleration().GetSafeNormal2D();
	}

	if (ReferenceDirection.IsNearlyZero())
	{
		ReferenceDirection = PlayerCharacter->GetActorForwardVector().GetSafeNormal2D();
	}

	if (ReferenceDirection.IsNearlyZero()) return nullptr;

	const FVector OwnerLocation = PlayerCharacter->GetActorLocation();
	const bool bDebug = RiftDebugCVars::IsTargetAssistDebugEnabled();
	if (bDebug)
	{
		DrawDebugLine(
			World,
			OwnerLocation,
			OwnerLocation + ReferenceDirection * MaxRange,
			FColor::Blue,
			false,
			0.05f,
			0,
			2.0f
		);
	}

	TArray<FOverlapResult> OverlapResults;
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(RiftTargetAssist), false, PlayerCharacter);
	const FCollisionShape QueryShape = FCollisionShape::MakeSphere(MaxRange);

	World->OverlapMultiByChannel(
		OverlapResults,
		OwnerLocation,
		FQuat::Identity,
		ECC_Pawn,
		QueryShape,
		QueryParams
	);

	AActor* BestTarget = nullptr;
	float BestScore = -FLT_MAX;

	for (const FOverlapResult& OverlapResult : OverlapResults)
	{
		AActor* CandidateActor = OverlapResult.GetActor();
		AEnemyCharacter* EnemyCharacter = Cast<AEnemyCharacter>(CandidateActor);
		if (!EnemyCharacter || CandidateActor == PlayerCharacter) continue;

		FVector ToTarget = EnemyCharacter->GetActorLocation() - OwnerLocation;
		ToTarget.Z = 0.0f;

		const float Distance = ToTarget.Size();
		if (Distance <= UE_KINDA_SMALL_NUMBER || Distance > MaxRange) continue;

		const FVector TargetDirection = ToTarget / Distance;
		const float Dot = FMath::Clamp(FVector::DotProduct(ReferenceDirection, TargetDirection), -1.0f, 1.0f);
		const float AngleDegrees = FMath::RadiansToDegrees(FMath::Acos(Dot));
		if (AngleDegrees > MaxAngleDegrees) continue;

		const float AngleScore = 1.0f - AngleDegrees / MaxAngleDegrees;
		const float DistanceScore = 1.0f - Distance / MaxRange;
		const float Score =
			TargetAssistAngleWeight * AngleScore +
			TargetAssistDistanceWeight * DistanceScore;

		if (bDebug)
		{
			DrawDebugLine(
				World,
				OwnerLocation,
				EnemyCharacter->GetActorLocation(),
				FColor::Yellow,
				false,
				0.05f,
				0,
				1.0f
			);

			DrawDebugString(
				World,
				EnemyCharacter->GetActorLocation() + FVector(0.0f, 0.0f, 100.0f),
				FString::Printf(TEXT("%.2f"), Score),
				nullptr,
				FColor::Yellow,
				0.05f,
				false
			);
		}

		if (Score > BestScore)
		{
			BestScore = Score;
			BestTarget = EnemyCharacter;
		}
	}

	if (bDebug && BestTarget)
	{
		DrawDebugLine(
			World,
			OwnerLocation,
			BestTarget->GetActorLocation(),
			FColor::Green,
			false,
			0.05f,
			0,
			3.0f
		);
	}

	return BestTarget;
}

bool URiftTargetAssistComponent::GetDesiredFacing(FRotator& OutFacing) const
{
	const AActor* OwnerActor = GetOwner();
	const AActor* SoftTarget = FindSoftTarget();
	if (!OwnerActor || !SoftTarget) return false;

	FVector ToTarget = SoftTarget->GetActorLocation() - OwnerActor->GetActorLocation();
	ToTarget.Z = 0.0f;

	if (ToTarget.IsNearlyZero()) return false;

	OutFacing = FRotator(0.0f, ToTarget.Rotation().Yaw, 0.0f);
	return true;
}
