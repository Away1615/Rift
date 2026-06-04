// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/Notifies/AnimNotifyState_GameplayTagWindow.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Components/SkeletalMeshComponent.h"

void UAnimNotifyState_GameplayTagWindow::NotifyBegin(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	float TotalDuration,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (!MeshComp || !StateTag.IsValid()) return;

	AActor* Owner = MeshComp->GetOwner();
	if (!Owner) return;

	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Owner);
	if (!ASC) return;

	ASC->AddLooseGameplayTag(StateTag);
}

void UAnimNotifyState_GameplayTagWindow::NotifyEnd(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (!MeshComp || !StateTag.IsValid()) return;

	AActor* Owner = MeshComp->GetOwner();
	if (!Owner) return;

	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Owner);
	if (!ASC) return;

	ASC->RemoveLooseGameplayTag(StateTag);
}
