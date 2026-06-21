#pragma once

#include "CoreMinimal.h"
#include "RiftPlayerHitReactionTypes.generated.h"

UENUM(BlueprintType)
enum class ERiftPlayerHitReaction : uint8
{
	None UMETA(DisplayName="None"),
	IndicatorOnly UMETA(DisplayName="Indicator Only"),
	LightHit UMETA(DisplayName="Light Hit"),
	HeavyHit UMETA(DisplayName="Heavy Hit")
};
