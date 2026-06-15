#pragma once

#include "CoreMinimal.h"
#include "RiftDamageReactionTypes.generated.h"

UENUM(BlueprintType)
enum class ERiftPlayerDamageReactionType : uint8
{
	None UMETA(DisplayName="None"),
	Light UMETA(DisplayName="Light"),
	Heavy UMETA(DisplayName="Heavy")
};
