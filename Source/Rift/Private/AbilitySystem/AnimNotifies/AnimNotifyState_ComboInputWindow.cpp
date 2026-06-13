// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/AnimNotifies/AnimNotifyState_ComboInputWindow.h"

#include "AbilitySystem/Abilities/GA_ComboAttack.h"
#include "Components/SkeletalMeshComponent.h"

void UAnimNotifyState_ComboInputWindow::NotifyBegin(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	float TotalDuration,
	const FAnimNotifyEventReference& EventReference
)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (!MeshComp) return;

	if (UGA_ComboAttack* ComboAttack = UGA_ComboAttack::FindActiveComboInstance(MeshComp->GetOwner()))
	{
		ComboAttack->OpenComboInputWindow();
	}
}

void UAnimNotifyState_ComboInputWindow::NotifyEnd(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference
)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (!MeshComp) return;

	if (UGA_ComboAttack* ComboAttack = UGA_ComboAttack::FindActiveComboInstance(MeshComp->GetOwner()))
	{
		ComboAttack->CloseComboInputWindow();
	}
}
