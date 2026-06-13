// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/AnimNotifies/AnimNotifyState_MeleeHitWindow.h"

#include "Combat/RiftWeaponTraceComponent.h"
#include "Components/SkeletalMeshComponent.h"

void UAnimNotifyState_MeleeHitWindow::NotifyBegin(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	float TotalDuration,
	const FAnimNotifyEventReference& EventReference
)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (!MeshComp) return;

	AActor* OwnerActor = MeshComp->GetOwner();
	if (!OwnerActor) return;

	if (URiftWeaponTraceComponent* WeaponTraceComponent = OwnerActor->FindComponentByClass<URiftWeaponTraceComponent>())
	{
		WeaponTraceComponent->StartHitWindow(WeaponSlot);
	}
}

void UAnimNotifyState_MeleeHitWindow::NotifyEnd(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference
)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (!MeshComp) return;

	AActor* OwnerActor = MeshComp->GetOwner();
	if (!OwnerActor) return;

	if (URiftWeaponTraceComponent* WeaponTraceComponent = OwnerActor->FindComponentByClass<URiftWeaponTraceComponent>())
	{
		WeaponTraceComponent->EndHitWindow(WeaponSlot);
	}
}
