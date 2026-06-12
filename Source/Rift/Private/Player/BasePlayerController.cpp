// Fill out your copyright notice in the Description page of Project Settings.

#include "Player/BasePlayerController.h"

#include "InputActionValue.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Character/PlayerCharacter.h"
#include "Data/Player/Input/PlayerInputConfig.h"

void ABasePlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Use Input Config reference
	if (!IsLocalController() || !DefaultInputConfig || !DefaultInputConfig->MappingContext) return;
	ULocalPlayer* LocalPlayer = GetLocalPlayer();
	if (!LocalPlayer) return;

	UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	if (!Subsystem) return;

	Subsystem->AddMappingContext(DefaultInputConfig->MappingContext, DefaultInputConfig->MappingPriority);
}

void ABasePlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent);

	if (!EnhancedInputComponent || !DefaultInputConfig) return;

	/*
	Player Input: Move
	-> Input Mapping Context MoveInput to MoveAction
	-> Enhanced Input notify MoveAction is Triggered
	-> Enhanced Input call function Bind by BindAction
	-> HandleMoveInput exec game logic
	*/

	// Bind Move
	if (DefaultInputConfig->MoveAction)
	{
		EnhancedInputComponent->BindAction(
			DefaultInputConfig->MoveAction,
			ETriggerEvent::Triggered, this,
			&ABasePlayerController::HandleMoveInput
			);

		EnhancedInputComponent->BindAction(
			DefaultInputConfig->MoveAction,
			ETriggerEvent::Completed, this,
			&ABasePlayerController::HandleMoveCompleted
);

		EnhancedInputComponent->BindAction(
			DefaultInputConfig->MoveAction,
			ETriggerEvent::Canceled, this,
			&ABasePlayerController::HandleMoveCompleted
		);
	}

	if (DefaultInputConfig->LookAction)
	{
		EnhancedInputComponent->BindAction(
			DefaultInputConfig->LookAction,
			ETriggerEvent::Triggered,
			this,
			&ABasePlayerController::HandleLookInput
		);
	}
}

APlayerCharacter* ABasePlayerController::GetPlayerCharacter() const
{
	return GetPawn<APlayerCharacter>();
}

void ABasePlayerController::HandleMoveInput(const FInputActionValue& InputActionValue)
{
	APlayerCharacter* PlayerCharacter = GetPawn<APlayerCharacter>();

	if (!PlayerCharacter) return;

	PlayerCharacter->HandleMove(InputActionValue.Get<FVector2D>());
}

void ABasePlayerController::HandleLookInput(const FInputActionValue& InputActionValue)
{
	const FVector2D LookValue = InputActionValue.Get<FVector2D>();

	// Rotate around Z axis
	AddYawInput(LookValue.X);
	// Rotate around Y axis
	AddPitchInput(LookValue.Y);
}

void ABasePlayerController::HandleMoveCompleted()
{
	APlayerCharacter* PlayerCharacter = GetPawn<APlayerCharacter>();
	if (!PlayerCharacter) return;

	PlayerCharacter->ClearMovementInputCache();
}
