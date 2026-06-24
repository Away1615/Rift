// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EnemyHealthBarWidget.generated.h"

class UAbilitySystemComponent;
struct FOnAttributeChangeData;

UCLASS()
class RIFT_API UEnemyHealthBarWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void InitializeFor(UAbilitySystemComponent* InASC);

protected:
	virtual void NativeDestruct() override;

	UFUNCTION(BlueprintImplementableEvent, Category="HUD")
	void OnHealthChanged(float Current, float Max);

	UFUNCTION(BlueprintImplementableEvent, Category="HUD")
	void OnPoiseChanged(float Current, float Max);

private:
	void BindToAbilitySystem(UAbilitySystemComponent* ASC);
	void RefreshAll();
	void HandleHealthChanged(const FOnAttributeChangeData& Data);
	void HandlePoiseChanged(const FOnAttributeChangeData& Data);

	TWeakObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;
	bool bInitialized = false;
};
