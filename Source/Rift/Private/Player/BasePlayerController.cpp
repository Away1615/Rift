// Fill out your copyright notice in the Description page of Project Settings.

#include "Player/BasePlayerController.h"

#include "AbilitySystem/RiftAbilitySystemComponent.h"
#include "AbilitySystem/RiftGameplayTags.h"
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

	if (DefaultInputConfig->PrimaryAttackAction)
	{
		EnhancedInputComponent->BindAction(
			DefaultInputConfig->PrimaryAttackAction,
			ETriggerEvent::Started,
			this,
			&ABasePlayerController::HandlePrimaryAttackInput
		);
	}

	if (DefaultInputConfig->SecondaryAttackAction)
	{
		EnhancedInputComponent->BindAction(
			DefaultInputConfig->SecondaryAttackAction,
			ETriggerEvent::Started,
			this,
			&ABasePlayerController::HandleSecondaryAttackInput
		);
	}

	if (DefaultInputConfig->PrimaryHeavyAction)
	{
		EnhancedInputComponent->BindAction(
			DefaultInputConfig->PrimaryHeavyAction,
			ETriggerEvent::Triggered,
			this,
			&ABasePlayerController::HandlePrimaryHeavyInput
		);
	}

	if (DefaultInputConfig->SecondaryHeavyAction)
	{
		EnhancedInputComponent->BindAction(
			DefaultInputConfig->SecondaryHeavyAction,
			ETriggerEvent::Triggered,
			this,
			&ABasePlayerController::HandleSecondaryHeavyInput
		);
	}

	if (DefaultInputConfig->CoreAction)
	{
		EnhancedInputComponent->BindAction(
			DefaultInputConfig->CoreAction,
			ETriggerEvent::Started,
			this,
			&ABasePlayerController::HandleCoreInput
		);
	}
}

APlayerCharacter* ABasePlayerController::GetPlayerCharacter() const
{
	return GetPawn<APlayerCharacter>();
}

void ABasePlayerController::HandleMoveInput(const FInputActionValue& InputActionValue)
{
	APlayerCharacter* PlayerCharacter = GetPlayerCharacter();

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

void ABasePlayerController::HandlePrimaryAttackInput(const FInputActionValue& InputActionValue)
{
	static_cast<void>(InputActionValue);

	if (!IsLocalController()) return;

	APlayerCharacter* PlayerCharacter = GetPlayerCharacter();
	if (!PlayerCharacter) return;

	URiftAbilitySystemComponent* AbilitySystemComponent =
		Cast<URiftAbilitySystemComponent>(PlayerCharacter->GetAbilitySystemComponent());
	if (!AbilitySystemComponent) return;

	AbilitySystemComponent->AbilityInputTagPressed(RiftGameplayTags::InputTag_Attack_Primary);
}

void ABasePlayerController::HandleSecondaryAttackInput(const FInputActionValue& InputActionValue)
{
	static_cast<void>(InputActionValue);

	if (!IsLocalController()) return;

	APlayerCharacter* PlayerCharacter = GetPlayerCharacter();
	if (!PlayerCharacter) return;

	URiftAbilitySystemComponent* AbilitySystemComponent =
		Cast<URiftAbilitySystemComponent>(PlayerCharacter->GetAbilitySystemComponent());
	if (!AbilitySystemComponent) return;

	AbilitySystemComponent->AbilityInputTagPressed(RiftGameplayTags::InputTag_Attack_Secondary);
}

void ABasePlayerController::HandlePrimaryHeavyInput(const FInputActionValue& InputActionValue)
{
	static_cast<void>(InputActionValue);

	if (!IsLocalController()) return;

	APlayerCharacter* PlayerCharacter = GetPlayerCharacter();
	if (!PlayerCharacter) return;

	URiftAbilitySystemComponent* AbilitySystemComponent =
		Cast<URiftAbilitySystemComponent>(PlayerCharacter->GetAbilitySystemComponent());
	if (!AbilitySystemComponent) return;

	AbilitySystemComponent->AbilityInputTagPressed(RiftGameplayTags::InputTag_Attack_PrimaryHeavy);
}

void ABasePlayerController::HandleSecondaryHeavyInput(const FInputActionValue& InputActionValue)
{
	static_cast<void>(InputActionValue);

	if (!IsLocalController()) return;

	APlayerCharacter* PlayerCharacter = GetPlayerCharacter();
	if (!PlayerCharacter) return;

	URiftAbilitySystemComponent* AbilitySystemComponent =
		Cast<URiftAbilitySystemComponent>(PlayerCharacter->GetAbilitySystemComponent());
	if (!AbilitySystemComponent) return;

	AbilitySystemComponent->AbilityInputTagPressed(RiftGameplayTags::InputTag_Attack_SecondaryHeavy);
}

void ABasePlayerController::HandleCoreInput(const FInputActionValue& InputActionValue)
{
	static_cast<void>(InputActionValue);

	if (!IsLocalController()) return;

	APlayerCharacter* PlayerCharacter = GetPlayerCharacter();
	if (!PlayerCharacter) return;

	URiftAbilitySystemComponent* AbilitySystemComponent =
		Cast<URiftAbilitySystemComponent>(PlayerCharacter->GetAbilitySystemComponent());
	if (!AbilitySystemComponent) return;

	AbilitySystemComponent->AbilityInputTagPressed(RiftGameplayTags::InputTag_Core);
}

void ABasePlayerController::HandleMoveCompleted()
{
	APlayerCharacter* PlayerCharacter = GetPlayerCharacter();
	if (!PlayerCharacter) return;

	PlayerCharacter->ClearMovementInputCache();
}
