#pragma once

#include "CoreMinimal.h"
#include "RiftHitReactionTypes.generated.h"

class AActor;

UENUM(BlueprintType)
enum class ERiftHitReactDirection : uint8
{
	Front UMETA(DisplayName="Front"),
	Back UMETA(DisplayName="Back")
};

RIFT_API ERiftHitReactDirection CalculateHitReactDirection(const AActor* TargetActor, const FVector& InstigatorLocation);
RIFT_API FName GetHitReactSectionName(ERiftHitReactDirection Direction);
