// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"
#include "RiftAbilitySystemComponent.generated.h"

UCLASS()
class RIFT_API URiftAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	URiftAbilitySystemComponent();

	FGameplayTag PressedInputTagThisFrame;

	void AbilityInputTagPressed(const FGameplayTag& InputTag);

	UFUNCTION(Server, Reliable)
	void ServerSetPressedInputTag(FGameplayTag InputTag);
};
