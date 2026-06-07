// Fill out your copyright notice in the Description page of Project Settings.

#include "Animation/Notify/AnimNotifyState_SendGameplayEventWindow.h"

#include "Tools/GameTools.h"

void UAnimNotifyState_SendGameplayEventWindow::NotifyBegin(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	float TotalDuration,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	GameTools::SendGameplayEvent(MeshComp, BeginEventTag);
}

void UAnimNotifyState_SendGameplayEventWindow::NotifyTick(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	float FrameDeltaTime,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);
	GameTools::SendGameplayEvent(MeshComp, TickEventTag);
}

void UAnimNotifyState_SendGameplayEventWindow::NotifyEnd(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);
	GameTools::SendGameplayEvent(MeshComp, EndEventTag);
}
