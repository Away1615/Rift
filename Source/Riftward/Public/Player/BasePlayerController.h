// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BasePlayerController.generated.h"

struct FInputActionValue;
class UMoveInputConfig;
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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input")
	TObjectPtr<UMoveInputConfig> DefaultMoveInputConfig;

private:
	void HandleMoveInput(const FInputActionValue& InputActionValue);
	void HandleMoveCompleted();
	void HandleLookInput(const FInputActionValue& InputActionValue);
	void HandleJumpStarted();
	void HandleJumpCompleted();
};
