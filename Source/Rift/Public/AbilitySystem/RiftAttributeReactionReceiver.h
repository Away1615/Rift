#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "RiftAttributeReactionReceiver.generated.h"

UINTERFACE(MinimalAPI)
class URiftAttributeReactionReceiver : public UInterface
{
	GENERATED_BODY()
};

class RIFT_API IRiftAttributeReactionReceiver
{
	GENERATED_BODY()

public:
	virtual void HandleAttributeDeath(AActor* DeathInstigator) {}
	virtual void HandleAttributePoiseHit(bool bPoiseBroken, AActor* DamageInstigator) {}
	virtual void HandleAttributeDamageNumber(float DamageAmount, bool bBlocked, const FVector& WorldLocation) {}
	virtual float GetAttributeBlockingPoiseDamageMultiplier() const { return 1.0f; }
};
