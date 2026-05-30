// Fill out your copyright notice in the Description page of Project Settings.

#include "Player/BasePlayerController.h"
#include "InputActionValue.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Character/PlayerCharacter.h"
#include "Input/MoveInputConfig.h"

void ABasePlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Use Input Config reference
	if (!IsLocalController() || !DefaultMoveInputConfig || !DefaultMoveInputConfig->MappingContext) return;
	ULocalPlayer* LocalPlayer = GetLocalPlayer();
	if (!LocalPlayer) return;

	UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	if (!Subsystem) return;

	Subsystem->AddMappingContext(DefaultMoveInputConfig->MappingContext, DefaultMoveInputConfig->MappingPriority);
}

void ABasePlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent);

	if (!EnhancedInputComponent || !DefaultMoveInputConfig) return;

	/*
	Player Input: Move
	-> Input Mapping Context MoveInput to MoveAction
	-> Enhanced Input notify MoveAction is Triggered
	-> Enhanced Input call function Bind by BindAction
	-> HandleMoveInput exec game logic
	*/

	// Bind Move
	if (DefaultMoveInputConfig->MoveAction)
	{
		EnhancedInputComponent->BindAction(
			DefaultMoveInputConfig->MoveAction,
			ETriggerEvent::Triggered, this,
			&ABasePlayerController::HandleMoveInput
			);

		EnhancedInputComponent->BindAction(
			DefaultMoveInputConfig->MoveAction,
			ETriggerEvent::Completed, this,
			&ABasePlayerController::HandleMoveCompleted
);

		EnhancedInputComponent->BindAction(
			DefaultMoveInputConfig->MoveAction,
			ETriggerEvent::Canceled, this,
			&ABasePlayerController::HandleMoveCompleted
		);
	}

	// Bind Look
	if (DefaultMoveInputConfig->LookAction)
	{
		EnhancedInputComponent->BindAction(
			DefaultMoveInputConfig->LookAction,
			ETriggerEvent::Triggered, this,
			&ABasePlayerController::HandleLookInput
		);
	}

	// Bind Jump
	if (DefaultMoveInputConfig->JumpAction)
	{
		EnhancedInputComponent->BindAction(
			DefaultMoveInputConfig->JumpAction,
			ETriggerEvent::Started, this,
			&ABasePlayerController::HandleJumpStarted
		);

		EnhancedInputComponent->BindAction(
			DefaultMoveInputConfig->JumpAction,
			ETriggerEvent::Completed, this,
			&ABasePlayerController::HandleJumpCompleted
		);
	}

}

void ABasePlayerController::HandleMoveInput(const FInputActionValue& InputActionValue)
{
	APlayerCharacter* PlayerCharacter = GetPawn<APlayerCharacter>();

	if (!PlayerCharacter) return;

	PlayerCharacter->HandleMove(InputActionValue.Get<FVector2D>());
}

void ABasePlayerController::HandleMoveCompleted()
{
	APlayerCharacter* PlayerCharacter = GetPawn<APlayerCharacter>();
	if (!PlayerCharacter) return;

	PlayerCharacter->HandleMove(FVector2D::ZeroVector);
}

void ABasePlayerController::HandleLookInput(const FInputActionValue& InputActionValue)
{
	const FVector2D LookValue = InputActionValue.Get<FVector2D>();
	AddYawInput(LookValue.X);
	AddPitchInput(LookValue.Y);
}

void ABasePlayerController::HandleJumpStarted()
{
	APlayerCharacter* PlayerCharacter = GetPawn<APlayerCharacter>();
	if (!PlayerCharacter) return;
	PlayerCharacter->HandleJumpStarted();
}

void ABasePlayerController::HandleJumpCompleted()
{
	APlayerCharacter* PlayerCharacter = GetPawn<APlayerCharacter>();
	if (!PlayerCharacter) return;
	PlayerCharacter->HandleJumpCompleted();
}
