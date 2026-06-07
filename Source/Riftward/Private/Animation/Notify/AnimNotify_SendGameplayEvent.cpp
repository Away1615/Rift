// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/Notify/AnimNotify_SendGameplayEvent.h"

#include "Tools/GameTools.h"

void UAnimNotify_SendGameplayEvent::Notify(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	GameTools::SendGameplayEvent(MeshComp, EventTag);
}
