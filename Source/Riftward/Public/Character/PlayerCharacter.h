// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraComponent.h"
#include "Character/BaseCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "PlayerCharacter.generated.h"

/**
 *
 */
UCLASS()
class RIFTWARD_API APlayerCharacter : public ABaseCharacter
{
	GENERATED_BODY()

public:
	APlayerCharacter();

	void HandleMove(const FVector2D& InputValue);

	virtual void Tick(float DeltaTime) override;
	virtual void PostInitializeComponents() override;

	// Getter
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	// On the server, called when this Pawn is controlled by the Controller
	virtual void PossessedBy(AController* NewController) override;

	// On the client, called when PlayerState is copied to this Pawn
	virtual void OnRep_PlayerState() override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera")
	TObjectPtr<USpringArmComponent> SpringArmComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera")
	TObjectPtr<UCameraComponent> CameraComponent;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Locomotion")
	FVector2D MovementInputVector = FVector2D::ZeroVector;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Locomotion")
	FVector WorldMoveDirection = FVector::ZeroVector;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Locomotion")
	bool bHasMovementInput = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion")
	FRotator MovementRotationRate = FRotator(0.0f, 1200.0f, 0.0f);

private:
	void InitPlayerProperties();
	void InitGasActorInfo();
	void InitCameraComponents();
	void InitMovementSettings() const;
	void ApplyMovementTuningSettings() const;
	void ApplyCameraRelativeMovementInput();
	void ClearMovementInput();

};
