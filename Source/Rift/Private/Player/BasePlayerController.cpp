// Fill out your copyright notice in the Description page of Project Settings.

#include "Player/BasePlayerController.h"

#include "AbilitySystem/RiftAbilitySystemComponent.h"
#include "AbilitySystem/RiftGameplayTags.h"
#include "Appearance/RiftPlayerAppearanceTypes.h"
#include "Camera/PlayerCameraManager.h"
#include "InputActionValue.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Character/PlayerCharacter.h"
#include "Core/RiftLobbyGameMode.h"
#include "Data/Player/Input/PlayerInputConfig.h"
#include "Data/Player/PlayerClassConfig.h"
#include "Engine/DataTable.h"
#include "Player/BasePlayerState.h"

void ABasePlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (PlayerCameraManager)
	{
		PlayerCameraManager->ViewPitchMin = -60.0f;
		PlayerCameraManager->ViewPitchMax = 25.0f;
	}

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
			ETriggerEvent::Triggered,
			this,
			&ABasePlayerController::HandlePrimaryAttackInput
		);
	}

	if (DefaultInputConfig->SecondaryAttackAction)
	{
		EnhancedInputComponent->BindAction(
			DefaultInputConfig->SecondaryAttackAction,
			ETriggerEvent::Triggered,
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

void ABasePlayerController::RequestSelectPlayerClass(UPlayerClassConfig* ClassConfig)
{
	if (HasAuthority())
	{
		Server_SelectPlayerClass_Implementation(ClassConfig);
		return;
	}

	Server_SelectPlayerClass(ClassConfig);
}

void ABasePlayerController::RequestStartLobbyGame()
{
	if (HasAuthority())
	{
		Server_StartLobbyGame_Implementation();
		return;
	}

	Server_StartLobbyGame();
}

void ABasePlayerController::RequestFinishCharacterCreation()
{
	RequestFinishCharacterCreationWithAppearance(FRiftPlayerAppearanceSelection());
}

void ABasePlayerController::RequestFinishCharacterCreationWithAppearance(
	const FRiftPlayerAppearanceSelection& FinalAppearanceSelection)
{
	if (HasAuthority())
	{
		Server_RequestFinishCharacterCreationWithAppearance_Implementation(FinalAppearanceSelection);
		return;
	}

	Server_RequestFinishCharacterCreationWithAppearance(FinalAppearanceSelection);
}

void ABasePlayerController::Server_SelectPlayerClass_Implementation(UPlayerClassConfig* ClassConfig)
{
	if (!ClassConfig)
	{
		return;
	}

	ABasePlayerState* RiftPlayerState = GetPlayerState<ABasePlayerState>();
	if (!RiftPlayerState)
	{
		return;
	}

	RiftPlayerState->SetSelectedPlayerClassConfig(ClassConfig);
}

void ABasePlayerController::Server_StartLobbyGame_Implementation()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	ARiftLobbyGameMode* LobbyGameMode = World->GetAuthGameMode<ARiftLobbyGameMode>();
	if (!LobbyGameMode)
	{
		return;
	}

	LobbyGameMode->StartGameFromLobby(this);
}

void ABasePlayerController::Server_RequestFinishCharacterCreation_Implementation()
{
	Server_RequestFinishCharacterCreationWithAppearance_Implementation(FRiftPlayerAppearanceSelection());
}

void ABasePlayerController::Server_RequestFinishCharacterCreationWithAppearance_Implementation(
	FRiftPlayerAppearanceSelection FinalAppearanceSelection)
{
	ABasePlayerState* RiftPlayerState = GetPlayerState<ABasePlayerState>();
	if (!RiftPlayerState)
	{
		return;
	}

	UWorld* World = GetWorld();
	ARiftLobbyGameMode* LobbyGameMode = World ? World->GetAuthGameMode<ARiftLobbyGameMode>() : nullptr;
	UPlayerClassConfig* DefaultPlayerClassConfig = LobbyGameMode
		? LobbyGameMode->GetDefaultPlayerClassConfig()
		: nullptr;
	if (!DefaultPlayerClassConfig)
	{
		Client_LobbyActionFailed(TEXT("Player class is not configured."));
		return;
	}

	if (!RiftPlayerState->GetSelectedPlayerClassConfig())
	{
		RiftPlayerState->SetSelectedPlayerClassConfig(DefaultPlayerClassConfig);
	}

	if (!IsAppearanceSelectionValid(FinalAppearanceSelection))
	{
		Client_LobbyActionFailed(TEXT("Invalid appearance selection."));
		return;
	}

	RiftPlayerState->SetConfirmedAppearanceSelection(FinalAppearanceSelection);
	RiftPlayerState->SetLobbyCharacterConfirmed(true);

	if (LobbyGameMode)
	{
		LobbyGameMode->RefreshAllPlayersReady();
	}
}

void ABasePlayerController::Client_PlayLobbyStartTransition_Implementation(const float Duration)
{
	OnLobbyStartTransitionRequested.Broadcast(Duration);
	OnLobbyStartTransition(Duration);
}

void ABasePlayerController::Client_LobbyActionFailed_Implementation(const FString& ErrorMessage)
{
	OnLobbyActionFailedRequested.Broadcast(ErrorMessage);
	OnLobbyActionFailed(ErrorMessage);
}

void ABasePlayerController::GetAppearanceOptionsForSlot(
	const ERiftPlayerAppearanceSlot Slot,
	TArray<FName>& OutPartIds) const
{
	OutPartIds.Empty();

	if (!PlayerAppearanceTable)
	{
		return;
	}

	for (const FName& RowName : PlayerAppearanceTable->GetRowNames())
	{
		const FRiftPlayerAppearancePartRow* Row =
			PlayerAppearanceTable->FindRow<FRiftPlayerAppearancePartRow>(RowName, TEXT(""), false);

		if (Row && Row->Slot == Slot)
		{
			OutPartIds.Add(RowName);
		}
	}
}

FText ABasePlayerController::GetAppearancePartDisplayName(const FName PartId) const
{
	if (PartId == NAME_None || !PlayerAppearanceTable)
	{
		return FText::GetEmpty();
	}

	const FRiftPlayerAppearancePartRow* Row =
		PlayerAppearanceTable->FindRow<FRiftPlayerAppearancePartRow>(PartId, TEXT(""), false);

	return Row ? Row->DisplayName : FText::GetEmpty();
}

bool ABasePlayerController::IsAppearancePartValid(
	const ERiftPlayerAppearanceSlot Slot,
	const FName PartId) const
{
	if (PartId == NAME_None)
	{
		return true;
	}

	if (!PlayerAppearanceTable)
	{
		return false;
	}

	const FRiftPlayerAppearancePartRow* Row =
		PlayerAppearanceTable->FindRow<FRiftPlayerAppearancePartRow>(PartId, TEXT(""), false);
	return Row && Row->Slot == Slot;
}

bool ABasePlayerController::IsAppearanceSelectionValid(
	const FRiftPlayerAppearanceSelection& Selection) const
{
	return
		IsAppearancePartValid(ERiftPlayerAppearanceSlot::Hair, Selection.HairId) &&
		IsAppearancePartValid(ERiftPlayerAppearanceSlot::ArmUpperLeft, Selection.ArmUpperLeftId) &&
		IsAppearancePartValid(ERiftPlayerAppearanceSlot::ArmUpperRight, Selection.ArmUpperRightId);
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
