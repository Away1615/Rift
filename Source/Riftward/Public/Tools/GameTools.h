#pragma once

#include "CoreMinimal.h"

class USkeletalMeshComponent;
struct FGameplayTag;

namespace GameTools
{
	void SendGameplayEvent(const USkeletalMeshComponent* MeshComp, const FGameplayTag& EventTag, float EventMagnitude = 0.0f);
}
