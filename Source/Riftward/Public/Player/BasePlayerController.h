// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/PlayerCharacter.h"
#include "GameFramework/PlayerController.h"
#include "Data/Player/Input/PlayerInputConfig.h"
#include "BasePlayerController.generated.h"

class UBaseAbilitySystemComponent;
struct FInputActionValue;
class UPlayerInputConfig;
/**
 *
 */
UCLASS()
class RIFTWARD_API ABasePlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

	UBaseAbilitySystemComponent* GetBaseAbilitySystemComponent() const;
	APlayerCharacter* GetPlayerCharacter() const;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input")
	TObjectPtr<UPlayerInputConfig> DefaultInputConfig;

	void HandleAbilityInputPressed(EAbilityInputID AbilityInputID);
	void HandleAbilityInputReleased(EAbilityInputID AbilityInputID);

private:
	void HandleMoveInput(const FInputActionValue& InputActionValue);
	void HandleLookInput(const FInputActionValue& InputActionValue);
	void HandleMoveCompleted();
};
