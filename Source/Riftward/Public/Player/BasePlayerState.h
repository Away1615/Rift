// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystem/BaseAbilitySystemComponent.h"
#include "GameFramework/PlayerState.h"
#include "BasePlayerState.generated.h"

class UResourceAttributeSet;
class UHealthAttributeSet;
/**
 *
 */
UCLASS()
class RIFTWARD_API ABasePlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	ABasePlayerState();
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UHealthAttributeSet* GetHealthAttributeSet() const { return HealthAttributeSet; }
	UResourceAttributeSet* GetResourceAttributeSet() const { return ResourceAttributeSet; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Abilities")
	TObjectPtr<UBaseAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Abilities")
	TObjectPtr<UHealthAttributeSet> HealthAttributeSet;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Abilities")
	TObjectPtr<UResourceAttributeSet> ResourceAttributeSet;

};
