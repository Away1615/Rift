#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TimerManager.h"
#include "RiftCombatFeedbackComponent.generated.h"

UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class RIFT_API URiftCombatFeedbackComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	URiftCombatFeedbackComponent();

	// Called after server hit confirmation: local hit stop on all clients, camera shake on the attacker's client.
	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_PlayMeleeHitFeedback(AActor* HitEnemy);

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void ApplyHitStopToActor(AActor* TargetActor, float TimeDilation, float Duration);
	void RestoreHitStopForActor(TWeakObjectPtr<AActor> TargetActor);

	TMap<TWeakObjectPtr<AActor>, FTimerHandle> HitStopTimerHandles;
};
