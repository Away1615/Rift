// Fill out your copyright notice in the Description page of Project Settings.

#include "Camera/RiftHitCameraShake.h"

void URiftHitCameraShakePattern::GetShakePatternInfoImpl(FCameraShakeInfo& OutInfo) const
{
	OutInfo.Duration = FCameraShakeDuration(Duration);
}

void URiftHitCameraShakePattern::StartShakePatternImpl(const FCameraShakePatternStartParams& Params)
{
	ElapsedTime = 0.0f;
}

void URiftHitCameraShakePattern::UpdateShakePatternImpl(const FCameraShakePatternUpdateParams& Params, FCameraShakePatternUpdateResult& OutResult)
{
	ElapsedTime += Params.DeltaTime;

	const float Alpha = Duration > 0.0f ? FMath::Clamp(ElapsedTime / Duration, 0.0f, 1.0f) : 1.0f;
	const float Decay = FMath::Pow(1.0f - Alpha, DecayExponent);
	const float Scale = Params.GetTotalScale() * Decay;

	const float LocOsc = FMath::Sin(2.0f * UE_PI * LocationFrequency * ElapsedTime);
	OutResult.Location = FVector(0.0f, ShakeDirection.X, ShakeDirection.Y) * (LocationAmplitude * LocOsc * Scale);

	const float RollOsc = FMath::Sin(2.0f * UE_PI * RollFrequency * ElapsedTime);
	OutResult.Rotation = FRotator(0.0f, 0.0f, RollAmplitude * RollOsc * Scale);
	OutResult.Flags = ECameraShakePatternUpdateResultFlags::SkipAutoScale;
}

bool URiftHitCameraShakePattern::IsFinishedImpl() const
{
	return ElapsedTime >= Duration;
}

URiftHitCameraShake::URiftHitCameraShake(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<URiftHitCameraShakePattern>(TEXT("RootShakePattern")))
{
}
