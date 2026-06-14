#include "Combat/RiftCombatFeedbackComponent.h"

#include "Camera/PlayerCameraManager.h"
#include "Camera/RiftHitCameraShake.h"
#include "Character/PlayerCharacter.h"
#include "Combat/RiftWeaponTraceComponent.h"
#include "Data/Player/Combat/PlayerCombatConfig.h"
#include "Data/Player/PlayerClassConfig.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "TimerManager.h"

URiftCombatFeedbackComponent::URiftCombatFeedbackComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void URiftCombatFeedbackComponent::Multicast_PlayMeleeHitFeedback_Implementation(AActor* HitEnemy)
{
	APlayerCharacter* OwnerCharacter = Cast<APlayerCharacter>(GetOwner());
	if (!OwnerCharacter) return;

	const UPlayerClassConfig* ClassConfig = OwnerCharacter->GetPlayerClassConfig();
	const UPlayerCombatConfig* CombatConfig = ClassConfig ? ClassConfig->PlayerCombatConfig : nullptr;
	if (!CombatConfig) return;

	const float TimeDilation = FMath::Clamp(CombatConfig->HitStopTimeDilation, 0.01f, 1.0f);
	const float Duration = FMath::Max(0.0f, CombatConfig->HitStopDuration);
	if (Duration > 0.0f)
	{
		ApplyHitStopToActor(OwnerCharacter, TimeDilation, Duration);

		if (HitEnemy)
		{
			ApplyHitStopToActor(HitEnemy, TimeDilation, Duration);
		}
	}

	if (!OwnerCharacter->IsLocallyControlled()) return;

	APlayerController* PlayerController = Cast<APlayerController>(OwnerCharacter->GetController());
	if (!PlayerController || !PlayerController->PlayerCameraManager) return;

	URiftWeaponTraceComponent* WeaponTraceComponent = OwnerCharacter->GetWeaponTraceComponent();
	if (!WeaponTraceComponent) return;

	TSubclassOf<UCameraShakeBase> CameraShakeClass = WeaponTraceComponent->GetIncomingCameraShake();
	FVector2D ShakeDirection = WeaponTraceComponent->GetIncomingCameraShakeDir();
	if (!CameraShakeClass) return;

	UCameraShakeBase* CameraShake = PlayerController->PlayerCameraManager->StartCameraShake(CameraShakeClass, 1.0f);
	if (CameraShake)
	{
		if (URiftHitCameraShakePattern* Pattern = Cast<URiftHitCameraShakePattern>(CameraShake->GetRootShakePattern()))
		{
			Pattern->SetShakeDirection(ShakeDirection);
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

void URiftCombatFeedbackComponent::ApplyHitStopToActor(
	AActor* TargetActor,
	const float TimeDilation,
	const float Duration
)
{
	if (!TargetActor) return;

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
