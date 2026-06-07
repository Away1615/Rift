// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "Data/Player/Input/AbilityInputID.h"
#include "BaseAbilitySystemComponent.generated.h"

/**
 *
 */
UCLASS()
class RIFTWARD_API UBaseAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	bool TryBufferAttackInputForActiveCombo(EAbilityInputID NewInputID);
	void CancelActiveAbilitiesInterruptedByInput(EAbilityInputID NewInputID);

private:
	UFUNCTION(Server, Reliable)
	void ServerBufferAttackInputForActiveCombo(EAbilityInputID NewInputID);

	bool BufferAttackInputForActiveCombo(EAbilityInputID NewInputID);
	bool HasActiveAbilityForInput(EAbilityInputID InputID);
};
