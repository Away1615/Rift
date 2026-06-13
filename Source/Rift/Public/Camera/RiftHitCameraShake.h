// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraShakeBase.h"
#include "RiftHitCameraShake.generated.h"

UCLASS(BlueprintType, EditInlineNew, meta=(AutoExpandCategories="Shake"))
class RIFT_API URiftHitCameraShakePattern : public UCameraShakePattern
{
	GENERATED_BODY()

public:
	void SetShakeDirection(const FVector2D& InDir) { ShakeDirection = InDir.GetSafeNormal(); }

	UPROPERTY(EditAnywhere, Category="Shake")
	float Duration = 0.22f;

	UPROPERTY(EditAnywhere, Category="Shake|Location")
	float LocationAmplitude = 14.0f;

	UPROPERTY(EditAnywhere, Category="Shake|Location")
	float LocationFrequency = 24.0f;

	UPROPERTY(EditAnywhere, Category="Shake|Rotation")
	float RollAmplitude = 0.0f;

	UPROPERTY(EditAnywhere, Category="Shake|Rotation")
	float RollFrequency = 16.0f;

	UPROPERTY(EditAnywhere, Category="Shake|Decay")
	float DecayExponent = 2.5f;

private:
	virtual void GetShakePatternInfoImpl(FCameraShakeInfo& OutInfo) const override;
	virtual void StartShakePatternImpl(const FCameraShakePatternStartParams& Params) override;
	virtual void UpdateShakePatternImpl(const FCameraShakePatternUpdateParams& Params, FCameraShakePatternUpdateResult& OutResult) override;
	virtual bool IsFinishedImpl() const override;

	float ElapsedTime = 0.0f;
	FVector2D ShakeDirection = FVector2D(1.0f, 0.0f);
};

UCLASS(BlueprintType)
class RIFT_API URiftHitCameraShake : public UCameraShakeBase
{
	GENERATED_BODY()

public:
	URiftHitCameraShake(const FObjectInitializer& ObjectInitializer);
};
