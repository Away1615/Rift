// Fill out your copyright notice in the Description page of Project Settings.

#include "Data/Combat/RiftCombatCueConfig.h"

#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"

void URiftCombatCueConfig::PlayHit(UWorld* World, const FVector& Location, const FVector& Normal) const
{
	PlayImpact(World, HitNiagara, HitSound, Location, Normal);
}

void URiftCombatCueConfig::PlayBlocked(UWorld* World, const FVector& Location, const FVector& Normal) const
{
	PlayImpact(World, BlockedNiagara, BlockedSound, Location, Normal);
}

void URiftCombatCueConfig::PlayImpact(
	UWorld* World,
	UNiagaraSystem* Niagara,
	USoundBase* Sound,
	const FVector& Location,
	const FVector& Normal
) const
{
	if (!World)
	{
		return;
	}

	FVector ImpactNormal = Normal.GetSafeNormal();
	if (ImpactNormal.IsNearlyZero())
	{
		ImpactNormal = FVector::UpVector;
	}

	const FRotator ImpactRotation = bUseImpactNormalRotation
		? ImpactNormal.ToOrientationRotator()
		: FRotator::ZeroRotator;

	if (Niagara)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			World,
			Niagara,
			Location,
			ImpactRotation,
			FVector(FMath::Max(0.0f, NiagaraScale))
		);
	}

	if (Sound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			World,
			Sound,
			Location,
			FMath::Max(0.0f, VolumeMultiplier),
			FMath::Max(0.0f, PitchMultiplier)
		);
	}
}
