#include "Combat/RiftCombatFeedbackComponent.h"

#include "Camera/PlayerCameraManager.h"
#include "Camera/RiftHitCameraShake.h"
#include "Character/PlayerCharacter.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "TimerManager.h"

URiftCombatFeedbackComponent::URiftCombatFeedbackComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void URiftCombatFeedbackComponent::Multicast_PlayMeleeHitFeedback_Implementation(
	AActor* HitEnemy,
	const bool bPlayHitStop,
	const FRiftMeleeHitStopConfig HitStopConfig,
	const bool bPlayCameraShake,
	TSubclassOf<UCameraShakeBase> CameraShakeClass,
	const FVector2D CameraShakeDir
)
{
	APlayerCharacter* OwnerCharacter = Cast<APlayerCharacter>(GetOwner());
	if (!OwnerCharacter) return;

	if (bPlayHitStop && HitStopConfig.bEnableHitStop)
	{
		const float TimeDilation = FMath::Clamp(HitStopConfig.TimeDilation, 0.01f, 1.0f);
		const float Duration = FMath::Max(0.0f, HitStopConfig.Duration);
		if (Duration > 0.0f)
		{
			ApplyHitStopToActor(OwnerCharacter, TimeDilation, Duration);

			if (HitEnemy)
			{
				ApplyHitStopToActor(HitEnemy, TimeDilation, Duration);
			}
		}
	}

	if (!bPlayCameraShake) return;
	if (!OwnerCharacter->IsLocallyControlled()) return;

	APlayerController* PlayerController = Cast<APlayerController>(OwnerCharacter->GetController());
	if (!PlayerController || !PlayerController->PlayerCameraManager) return;

	if (!CameraShakeClass) return;

	UCameraShakeBase* CameraShake = PlayerController->PlayerCameraManager->StartCameraShake(CameraShakeClass, 1.0f);
	if (CameraShake)
	{
		if (URiftHitCameraShakePattern* Pattern = Cast<URiftHitCameraShakePattern>(CameraShake->GetRootShakePattern()))
		{
			Pattern->SetShakeDirection(CameraShakeDir);
		}
	}
}

void URiftCombatFeedbackComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		for (auto& Pair : HitStopTimerHandles)
		{
			World->GetTimerManager().ClearTimer(Pair.Value);

			if (AActor* Target = Pair.Key.Get())
			{
				Target->CustomTimeDilation = 1.0f;
			}
		}
	}

	HitStopTimerHandles.Empty();
	Super::EndPlay(EndPlayReason);
}

void URiftCombatFeedbackComponent::	ApplyHitStopToActor(
	AActor* TargetActor,
	const float TimeDilation,
	const float Duration
)
{
	if (!TargetActor) return;

	if (!FMath::IsNearlyEqual(TargetActor->CustomTimeDilation, 1.0f)) return;

	UWorld* World = GetWorld();
	if (!World) return;

	TWeakObjectPtr<AActor> WeakTarget(TargetActor);

	if (FTimerHandle* ExistingHandle = HitStopTimerHandles.Find(WeakTarget))
	{
		World->GetTimerManager().ClearTimer(*ExistingHandle);
	}

	TargetActor->CustomTimeDilation = TimeDilation;

	FTimerDelegate TimerDelegate;
	TimerDelegate.BindUObject(this, &URiftCombatFeedbackComponent::RestoreHitStopForActor, WeakTarget);

	FTimerHandle& TimerHandle = HitStopTimerHandles.FindOrAdd(WeakTarget);
	World->GetTimerManager().SetTimer(TimerHandle, TimerDelegate, Duration, false);
}

void URiftCombatFeedbackComponent::RestoreHitStopForActor(TWeakObjectPtr<AActor> TargetActor)
{
	if (AActor* RestoredActor = TargetActor.Get())
	{
		RestoredActor->CustomTimeDilation = 1.0f;
	}

	HitStopTimerHandles.Remove(TargetActor);
}
