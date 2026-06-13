// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/AnimNotifies/AnimNotify_ComboChainPoint.h"

#include "AbilitySystem/Abilities/GA_ComboAttack.h"
#include "Components/SkeletalMeshComponent.h"

void UAnimNotify_ComboChainPoint::Notify(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference
)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!MeshComp) return;

	if (UGA_ComboAttack* ComboAttack = UGA_ComboAttack::FindActiveComboInstance(MeshComp->GetOwner()))
	{
		ComboAttack->CommitComboChainPoint();
	}
}
