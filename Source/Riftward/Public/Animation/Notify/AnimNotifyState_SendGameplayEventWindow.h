// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "GameplayTagContainer.h"
#include "AnimNotifyState_SendGameplayEventWindow.generated.h"

UCLASS(DisplayName="GAS | Event Window")
class RIFTWARD_API UAnimNotifyState_SendGameplayEventWindow : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Gameplay Event")
	FGameplayTag BeginEventTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Gameplay Event")
	FGameplayTag TickEventTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Gameplay Event")
	FGameplayTag EndEventTag;

	virtual void NotifyBegin(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		float TotalDuration,
		const FAnimNotifyEventReference& EventReference) override;

	virtual void NotifyTick(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		float FrameDeltaTime,
		const FAnimNotifyEventReference& EventReference) override;

	virtual void NotifyEnd(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;
};
