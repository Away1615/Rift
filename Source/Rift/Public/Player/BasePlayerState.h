// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/PlayerState.h"
#include "BasePlayerState.generated.h"

class URiftAbilitySystemComponent;
class URiftAttributeSet;

/**
 *
 */
UCLASS()
class RIFT_API ABasePlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	ABasePlayerState();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="AbilitySystem")
	TObjectPtr<URiftAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="AbilitySystem")
	TObjectPtr<URiftAttributeSet> AttributeSet;
};
