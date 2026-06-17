// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/GameplayCues/RiftGameplayCueNotify_CombatImpact.h"

#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"

bool URiftGameplayCueNotify_CombatImpact::OnExecute_Implementation(
	AActor* MyTarget,
	const FGameplayCueParameters& Parameters
) const
{
	if (!MyTarget)
	{
		return true;
	}

	FVector ImpactLocation = Parameters.Location;
	if (ImpactLocation.IsNearlyZero())
	{
		ImpactLocation = MyTarget->GetActorLocation();
	}

	FVector ImpactNormal = Parameters.Normal.GetSafeNormal();
	if (ImpactNormal.IsNearlyZero())
	{
		ImpactNormal = FVector::UpVector;
	}

	const FRotator ImpactRotation = bUseImpactNormalRotation
		? ImpactNormal.ToOrientationRotator()
		: FRotator::ZeroRotator;

	if (ImpactNiagara)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			MyTarget,
			ImpactNiagara,
			ImpactLocation,
			ImpactRotation,
			FVector(FMath::Max(0.0f, NiagaraScale))
		);
	}

	if (ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			MyTarget,
			ImpactSound,
			ImpactLocation,
			FMath::Max(0.0f, VolumeMultiplier),
			FMath::Max(0.0f, PitchMultiplier)
		);
	}

	return true;
}
