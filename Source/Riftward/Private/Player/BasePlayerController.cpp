// Fill out your copyright notice in the Description page of Project Settings.

#include "Player/BasePlayerController.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystem/BaseAbilitySystemComponent.h"
#include "InputActionValue.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Character/PlayerCharacter.h"
#include "Input/PlayerInputConfig.h"

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

	// Bind Look
	if (DefaultInputConfig->LookAction)
	{
		EnhancedInputComponent->BindAction(
			DefaultInputConfig->LookAction,
			ETriggerEvent::Triggered, this,
			&ABasePlayerController::HandleLookInput
		);
	}

	// Bind Ability Inputs
	for (const FAbilityInputAction& Action : DefaultInputConfig->AbilityInputActions)
	{
		if (!Action.InputAction || Action.InputID == EAbilityInputID::None)
		{
			continue;
		}

		EnhancedInputComponent->BindAction(
			Action.InputAction,
			ETriggerEvent::Started,
			this,
			&ABasePlayerController::HandleAbilityInputPressed,
			Action.InputID
		);

		EnhancedInputComponent->BindAction(
			Action.InputAction,
			ETriggerEvent::Completed,
			this,
			&ABasePlayerController::HandleAbilityInputReleased,
			Action.InputID
		);

		EnhancedInputComponent->BindAction(
			Action.InputAction,
			ETriggerEvent::Canceled,
			this,
			&ABasePlayerController::HandleAbilityInputReleased,
			Action.InputID
		);
	}

}

UBaseAbilitySystemComponent* ABasePlayerController::GetBaseAbilitySystemComponent() const
{
	const APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn) return nullptr;

	const IAbilitySystemInterface* AbilitySystemInterface = Cast<IAbilitySystemInterface>(ControlledPawn);
	if (!AbilitySystemInterface) return nullptr;

	return Cast<UBaseAbilitySystemComponent>(AbilitySystemInterface->GetAbilitySystemComponent());
}

void ABasePlayerController::HandleAbilityInputPressed(EAbilityInputID AbilityInputID)
{
	if (AbilityInputID == EAbilityInputID::None) return;

	UBaseAbilitySystemComponent* ASC = GetBaseAbilitySystemComponent();
	if (!ASC) return;

	ASC->AbilityLocalInputPressed(static_cast<int32>(AbilityInputID));
}

void ABasePlayerController::HandleAbilityInputReleased(EAbilityInputID AbilityInputID)
{
	if (AbilityInputID == EAbilityInputID::None) return;

	UBaseAbilitySystemComponent* ASC = GetBaseAbilitySystemComponent();
	if (!ASC) return;
	ASC->AbilityLocalInputReleased(static_cast<int32>(AbilityInputID));
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
