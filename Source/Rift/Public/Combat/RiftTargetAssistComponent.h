// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RiftTargetAssistComponent.generated.h"

/**
 * Soft target helper for combat abilities.
 *
 * Contract: this component only outputs a SoftTarget or DesiredFacing.
 * It must not modify Controller Rotation, control the camera, or implement hard lock-on.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class RIFT_API URiftTargetAssistComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	URiftTargetAssistComponent();

	AActor* FindSoftTarget() const;
	bool GetDesiredFacing(FRotator& OutFacing) const;

private:
	UPROPERTY(EditDefaultsOnly, Category="TargetAssist")
	float TargetAssistMaxRange = 800.0f;

	UPROPERTY(EditDefaultsOnly, Category="TargetAssist")
	float TargetAssistMaxAngleDegrees = 75.0f;

	UPROPERTY(EditDefaultsOnly, Category="TargetAssist")
	float TargetAssistAngleWeight = 0.7f;

	UPROPERTY(EditDefaultsOnly, Category="TargetAssist")
	float TargetAssistDistanceWeight = 0.3f;
};
