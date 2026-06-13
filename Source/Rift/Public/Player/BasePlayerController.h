// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/PlayerCharacter.h"
#include "GameFramework/PlayerController.h"
#include "Data/Player/Input/PlayerInputConfig.h"
#include "BasePlayerController.generated.h"

struct FInputActionValue;
class UPlayerInputConfig;
/**
 *
 */
UCLASS()
class RIFT_API ABasePlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

	APlayerCharacter* GetPlayerCharacter() const;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input")
	TObjectPtr<UPlayerInputConfig> DefaultInputConfig;

private:
	void HandleMoveInput(const FInputActionValue& InputActionValue);
	void HandleLookInput(const FInputActionValue& InputActionValue);
	void HandlePrimaryAttackInput(const FInputActionValue& InputActionValue);
	void HandleMoveCompleted();
};
