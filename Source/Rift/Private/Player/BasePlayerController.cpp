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
#include "Core/RiftGameInstance.h"
#include "Data/Player/Input/PlayerInputConfig.h"
#include "Data/Player/PlayerClassConfig.h"
#include "Engine/DataTable.h"
#include "Player/BasePlayerState.h"

static bool IsAbilityWithTagActive(
	URiftAbilitySystemComponent* AbilitySystemComponent,
	const FGameplayTag& AbilityTag
)
{
	if (!AbilitySystemComponent || !AbilityTag.IsValid()) return false;

	FScopedAbilityListLock AbilityListLock(*AbilitySystemComponent);
	for (const FGameplayAbilitySpec& AbilitySpec : AbilitySystemComponent->GetActivatableAbilities())
	{
		if (!AbilitySpec.IsActive() || !AbilitySpec.Ability)
		{
			continue;
		}

		if (AbilitySpec.Ability->GetAssetTags().HasTagExact(AbilityTag))
		{
			return true;
		}
	}

	return false;
}

static bool IsPrimaryAttackAbilityActive(URiftAbilitySystemComponent* AbilitySystemComponent)
{
	return IsAbilityWithTagActive(AbilitySystemComponent, RiftGameplayTags::Ability_Attack_Combo) ||
		IsAbilityWithTagActive(AbilitySystemComponent, RiftGameplayTags::Ability_Attack_TwinSwordCombo) ||
		IsAbilityWithTagActive(AbilitySystemComponent, RiftGameplayTags::Ability_Attack_TwinSwordRapidSlash);
}

static bool IsGuardActive(URiftAbilitySystemComponent* AbilitySystemComponent)
{
	return AbilitySystemComponent &&
		(AbilitySystemComponent->HasMatchingGameplayTag(RiftGameplayTags::Ability_Guard) ||
			AbilitySystemComponent->HasMatchingGameplayTag(RiftGameplayTags::State_Blocking));
}

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

void ABasePlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
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

	if (DefaultInputConfig->DodgeAction)
	{
		EnhancedInputComponent->BindAction(
			DefaultInputConfig->DodgeAction,
			ETriggerEvent::Started,
			this,
			&ABasePlayerController::HandleDodgeInput
		);
	}

	if (DefaultInputConfig->GuardAction)
	{
		EnhancedInputComponent->BindAction(
			DefaultInputConfig->GuardAction,
			ETriggerEvent::Started,
			this,
			&ABasePlayerController::HandleGuardStarted
		);

		EnhancedInputComponent->BindAction(
			DefaultInputConfig->GuardAction,
			ETriggerEvent::Completed,
			this,
			&ABasePlayerController::HandleGuardCompleted
		);

		EnhancedInputComponent->BindAction(
			DefaultInputConfig->GuardAction,
			ETriggerEvent::Canceled,
			this,
			&ABasePlayerController::HandleGuardCompleted
		);
	}

	if (DefaultInputConfig->SkillQAction)
	{
		EnhancedInputComponent->BindAction(
			DefaultInputConfig->SkillQAction,
			ETriggerEvent::Started,
			this,
			&ABasePlayerController::HandleSkillQInput
		);
	}

	if (DefaultInputConfig->SkillEAction)
	{
		EnhancedInputComponent->BindAction(
			DefaultInputConfig->SkillEAction,
			ETriggerEvent::Started,
			this,
			&ABasePlayerController::HandleSkillEInput
		);
	}
}

void ABasePlayerController::RequestSelectPlayerClass(UPlayerClassConfig* ClassConfig)
{
	if (HasAuthority())
	{
		Server_RequestSelectPlayerClass_Implementation(ClassConfig);
		return;
	}

	Server_RequestSelectPlayerClass(ClassConfig);
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

void ABasePlayerController::Server_RequestSelectPlayerClass_Implementation(UPlayerClassConfig* ClassConfig)
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

	if (!IsAppearanceSelectionValid(FinalAppearanceSelection))
	{
		Client_LobbyActionFailed(TEXT("Invalid appearance selection."));
		return;
	}

	RiftPlayerState->SetConfirmedAppearanceSelection(FinalAppearanceSelection);
	RiftPlayerState->SetLobbyCharacterConfirmed(true);

	UWorld* World = GetWorld();
	ARiftLobbyGameMode* LobbyGameMode = World ? World->GetAuthGameMode<ARiftLobbyGameMode>() : nullptr;
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

void ABasePlayerController::Client_ShowVictory_Implementation()
{
	bShowMouseCursor = true;

	FInputModeUIOnly InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);

	OnVictory();
}

void ABasePlayerController::Client_EnterGameplayInputMode_Implementation()
{
	bShowMouseCursor = false;
	SetIgnoreMoveInput(false);
	SetIgnoreLookInput(false);

	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);
}

void ABasePlayerController::ReturnToMainMenuFromVictory()
{
	if (URiftGameInstance* RiftGameInstance = GetGameInstance<URiftGameInstance>())
	{
		RiftGameInstance->LeaveRoom();
	}

	if (!MainMenuMapName.IsNone())
	{
		ClientTravel(MainMenuMapName.ToString(), TRAVEL_Absolute);
	}
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

void ABasePlayerController::CacheLobbySelectionForGameplay(
	UPlayerClassConfig* ClassConfig,
	const FRiftPlayerAppearanceSelection& AppearanceSelection
)
{
	PendingGameplayPlayerClassConfig = ClassConfig;
	PendingGameplayAppearanceSelection = AppearanceSelection;

	const bool bHasConfirmedAppearance =
		!AppearanceSelection.HairId.IsNone() ||
		!AppearanceSelection.ArmUpperLeftId.IsNone() ||
		!AppearanceSelection.ArmUpperRightId.IsNone();
	UE_LOG(
		LogTemp,
		Warning,
		TEXT("RiftLobbySelection Cache Controller=%s Class=%s HasAppearance=%s Hair=%s ArmUpperLeft=%s ArmUpperRight=%s"),
		*GetNameSafe(this),
		*GetNameSafe(ClassConfig),
		bHasConfirmedAppearance ? TEXT("true") : TEXT("false"),
		*AppearanceSelection.HairId.ToString(),
		*AppearanceSelection.ArmUpperLeftId.ToString(),
		*AppearanceSelection.ArmUpperRightId.ToString()
	);
}

void ABasePlayerController::ApplyCachedLobbySelectionToPlayerState()
{
	ABasePlayerState* RiftPlayerState = GetPlayerState<ABasePlayerState>();
	if (!RiftPlayerState)
	{
		UE_LOG(LogTemp, Warning, TEXT("RiftLobbySelection ApplyCached skipped: Controller=%s has no BasePlayerState."), *GetNameSafe(this));
		return;
	}

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("RiftLobbySelection ApplyCached Controller=%s PlayerState=%s CurrentClass=%s PendingClass=%s"),
		*GetNameSafe(this),
		*GetNameSafe(RiftPlayerState),
		*GetNameSafe(RiftPlayerState->GetSelectedPlayerClassConfig()),
		*GetNameSafe(PendingGameplayPlayerClassConfig)
	);

	if (!PendingGameplayPlayerClassConfig)
	{
		return;
	}

	RiftPlayerState->SetSelectedPlayerClassConfig(PendingGameplayPlayerClassConfig);
	RiftPlayerState->SetConfirmedAppearanceSelection(PendingGameplayAppearanceSelection);
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

	if (PlayerCharacter->IsDodgingForActionCancel())
	{
		return;
	}

	if (IsGuardActive(AbilitySystemComponent))
	{
		PlayerCharacter->CancelPlayerActionAbilities(true, false);
	}
	else if (AbilitySystemComponent->HasMatchingGameplayTag(RiftGameplayTags::State_Attacking))
	{
		if (!PlayerCharacter->IsActionCancelable())
		{
			if (!IsPrimaryAttackAbilityActive(AbilitySystemComponent))
			{
				return;
			}
		}
		else if (!IsPrimaryAttackAbilityActive(AbilitySystemComponent))
		{
			PlayerCharacter->CancelPlayerActionAbilities(false, false);
		}
	}

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

	if (PlayerCharacter->IsDodgingForActionCancel())
	{
		return;
	}

	if (AbilitySystemComponent->HasMatchingGameplayTag(RiftGameplayTags::State_Attack_RapidSlash))
	{
		PlayerCharacter->RequestRapidSlashFinisher();
		return;
	}

	return;
}

void ABasePlayerController::HandleDodgeInput(const FInputActionValue& InputActionValue)
{
	static_cast<void>(InputActionValue);

	if (!IsLocalController()) return;

	APlayerCharacter* PlayerCharacter = GetPlayerCharacter();
	if (!PlayerCharacter) return;

	URiftAbilitySystemComponent* AbilitySystemComponent =
		Cast<URiftAbilitySystemComponent>(PlayerCharacter->GetAbilitySystemComponent());
	if (!AbilitySystemComponent) return;

	if (PlayerCharacter->IsDodgingForActionCancel())
	{
		return;
	}

	PlayerCharacter->CancelPlayerActionAbilities(true, false);
	AbilitySystemComponent->AbilityInputTagPressed(RiftGameplayTags::InputTag_Dodge);
}

void ABasePlayerController::HandleGuardStarted(const FInputActionValue& InputActionValue)
{
	static_cast<void>(InputActionValue);

	if (!IsLocalController()) return;

	APlayerCharacter* PlayerCharacter = GetPlayerCharacter();
	if (!PlayerCharacter) return;

	URiftAbilitySystemComponent* AbilitySystemComponent =
		Cast<URiftAbilitySystemComponent>(PlayerCharacter->GetAbilitySystemComponent());
	if (!AbilitySystemComponent) return;

	if (PlayerCharacter->IsDodgingForActionCancel())
	{
		return;
	}

	if (AbilitySystemComponent->HasMatchingGameplayTag(RiftGameplayTags::State_Attacking))
	{
		if (!PlayerCharacter->IsActionCancelable())
		{
			return;
		}

		PlayerCharacter->CancelPlayerActionAbilities(false, false);
	}

	AbilitySystemComponent->AbilityInputTagPressed(RiftGameplayTags::InputTag_Guard);
}

void ABasePlayerController::HandleGuardCompleted(const FInputActionValue& InputActionValue)
{
	static_cast<void>(InputActionValue);

	if (!IsLocalController()) return;

	APlayerCharacter* PlayerCharacter = GetPlayerCharacter();
	if (!PlayerCharacter) return;

	URiftAbilitySystemComponent* AbilitySystemComponent =
		Cast<URiftAbilitySystemComponent>(PlayerCharacter->GetAbilitySystemComponent());
	if (!AbilitySystemComponent) return;

	AbilitySystemComponent->AbilityInputTagReleased(RiftGameplayTags::InputTag_Guard);
}

void ABasePlayerController::HandleSkillQInput(const FInputActionValue& InputActionValue)
{
	static_cast<void>(InputActionValue);

	if (!IsLocalController()) return;

	APlayerCharacter* PlayerCharacter = GetPlayerCharacter();
	if (!PlayerCharacter) return;

	URiftAbilitySystemComponent* AbilitySystemComponent =
		Cast<URiftAbilitySystemComponent>(PlayerCharacter->GetAbilitySystemComponent());
	if (!AbilitySystemComponent) return;

	if (PlayerCharacter->IsDodgingForActionCancel())
	{
		return;
	}

	if (IsGuardActive(AbilitySystemComponent))
	{
		PlayerCharacter->CancelPlayerActionAbilities(true, false);
	}
	else if (AbilitySystemComponent->HasMatchingGameplayTag(RiftGameplayTags::State_Attacking))
	{
		if (!PlayerCharacter->IsActionCancelable())
		{
			return;
		}

		PlayerCharacter->CancelPlayerActionAbilities(false, false);
	}

	AbilitySystemComponent->AbilityInputTagPressed(RiftGameplayTags::InputTag_Skill_Q);
}

void ABasePlayerController::HandleSkillEInput(const FInputActionValue& InputActionValue)
{
	static_cast<void>(InputActionValue);

	if (!IsLocalController()) return;

	APlayerCharacter* PlayerCharacter = GetPlayerCharacter();
	if (!PlayerCharacter) return;

	URiftAbilitySystemComponent* AbilitySystemComponent =
		Cast<URiftAbilitySystemComponent>(PlayerCharacter->GetAbilitySystemComponent());
	if (!AbilitySystemComponent) return;

	if (PlayerCharacter->IsDodgingForActionCancel())
	{
		return;
	}

	if (IsGuardActive(AbilitySystemComponent))
	{
		PlayerCharacter->CancelPlayerActionAbilities(true, false);
	}
	else if (AbilitySystemComponent->HasMatchingGameplayTag(RiftGameplayTags::State_Attacking))
	{
		if (!PlayerCharacter->IsActionCancelable())
		{
			return;
		}

		PlayerCharacter->CancelPlayerActionAbilities(false, false);
	}

	AbilitySystemComponent->AbilityInputTagPressed(RiftGameplayTags::InputTag_Skill_E);
}

void ABasePlayerController::HandleMoveCompleted()
{
	APlayerCharacter* PlayerCharacter = GetPlayerCharacter();
	if (!PlayerCharacter) return;

	PlayerCharacter->ClearMovementInputCache();
}
