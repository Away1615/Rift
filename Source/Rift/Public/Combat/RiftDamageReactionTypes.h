#pragma once

#include "CoreMinimal.h"
#include "RiftDamageReactionTypes.generated.h"

UENUM(BlueprintType)
enum class ERiftPlayerHitFeedbackPolicy : uint8
{
	None UMETA(DisplayName="None"),
	FeedbackOnly UMETA(DisplayName="Feedback Only"),
	LightHit UMETA(DisplayName="Light Hit")
};
