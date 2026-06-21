#pragma once

#include "CoreMinimal.h"
#include "RiftCombatFeedbackTypes.generated.h"

USTRUCT(BlueprintType)
struct FRiftMeleeHitStopConfig
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="HitStop")
	bool bEnableHitStop = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="HitStop", meta=(EditCondition="bEnableHitStop", ClampMin="0.0"))
	float Duration = 0.04f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="HitStop", meta=(EditCondition="bEnableHitStop", ClampMin="0.01", ClampMax="1.0"))
	float TimeDilation = 0.1f;
};
