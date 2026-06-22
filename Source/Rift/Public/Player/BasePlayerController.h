// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Appearance/RiftPlayerAppearanceTypes.h"
#include "GameFramework/PlayerController.h"
#include "Data/Player/Input/PlayerInputConfig.h"
#include "BasePlayerController.generated.h"

struct FInputActionValue;
class APlayerCharacter;
class UDataTable;
class UPlayerInputConfig;
class UPlayerClassConfig;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FRiftLobbyStartTransitionSignature, float, Duration);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FRiftLobbyActionFailedSignature, const FString&, ErrorMessage);

/**
 *
 */
UCLASS()
class RIFT_API ABasePlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="Rift|Lobby")
	void RequestSelectPlayerClass(UPlayerClassConfig* ClassConfig);

	UFUNCTION(BlueprintCallable, Category="Rift|Lobby")
	void RequestStartLobbyGame();

	UFUNCTION(BlueprintCallable, Category="Rift|Lobby")
	void RequestFinishCharacterCreation();

	UFUNCTION(BlueprintCallable, Category="Rift|Lobby")
	void RequestFinishCharacterCreationWithAppearance(const FRiftPlayerAppearanceSelection& FinalAppearanceSelection);

	UFUNCTION(Server, Reliable)
	void Server_RequestSelectPlayerClass(UPlayerClassConfig* ClassConfig);

	UFUNCTION(Server, Reliable)
	void Server_StartLobbyGame();

	UFUNCTION(Server, Reliable)
	void Server_RequestFinishCharacterCreation();

	UFUNCTION(Server, Reliable)
	void Server_RequestFinishCharacterCreationWithAppearance(FRiftPlayerAppearanceSelection FinalAppearanceSelection);

	UFUNCTION(Client, Reliable)
	void Client_PlayLobbyStartTransition(float Duration);

	UFUNCTION(Client, Reliable)
	void Client_LobbyActionFailed(const FString& ErrorMessage);

	UFUNCTION(Client, Reliable)
	void Client_ShowVictory();

	UFUNCTION(BlueprintCallable, Category="Rift|Gameplay")
	void ReturnToMainMenuFromVictory();

	UPROPERTY(BlueprintAssignable, Category="Rift|Lobby")
	FRiftLobbyStartTransitionSignature OnLobbyStartTransitionRequested;

	UPROPERTY(BlueprintAssignable, Category="Rift|Lobby")
	FRiftLobbyActionFailedSignature OnLobbyActionFailedRequested;

	UFUNCTION(BlueprintImplementableEvent, Category="Rift|Lobby")
	void OnLobbyStartTransition(float Duration);

	UFUNCTION(BlueprintImplementableEvent, Category="Rift|Lobby")
	void OnLobbyActionFailed(const FString& ErrorMessage);

	UFUNCTION(BlueprintImplementableEvent, Category="Rift|Gameplay")
	void OnVictory();

	UFUNCTION(BlueprintCallable, Category="Rift|Appearance")
	void GetAppearanceOptionsForSlot(ERiftPlayerAppearanceSlot Slot, TArray<FName>& OutPartIds) const;

	UFUNCTION(BlueprintPure, Category="Rift|Appearance")
	FText GetAppearancePartDisplayName(FName PartId) const;

	void CacheLobbySelectionForGameplay(
		UPlayerClassConfig* ClassConfig,
		const FRiftPlayerAppearanceSelection& AppearanceSelection
	);
	void ApplyCachedLobbySelectionToPlayerState();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Appearance")
	TObjectPtr<UDataTable> PlayerAppearanceTable;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void SetupInputComponent() override;

	APlayerCharacter* GetPlayerCharacter() const;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input")
	TObjectPtr<UPlayerInputConfig> DefaultInputConfig;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Rift|Victory")
	FName MainMenuMapName = TEXT("/Game/0_/Map/MainMenuMap");

private:
	UPROPERTY()
	TObjectPtr<UPlayerClassConfig> PendingGameplayPlayerClassConfig;

	UPROPERTY()
	FRiftPlayerAppearanceSelection PendingGameplayAppearanceSelection;

	bool IsAppearancePartValid(ERiftPlayerAppearanceSlot Slot, FName PartId) const;
	bool IsAppearanceSelectionValid(const FRiftPlayerAppearanceSelection& Selection) const;

	void HandleMoveInput(const FInputActionValue& InputActionValue);
	void HandleLookInput(const FInputActionValue& InputActionValue);
	void HandlePrimaryAttackInput(const FInputActionValue& InputActionValue);
	void HandleSecondaryAttackInput(const FInputActionValue& InputActionValue);
	void HandleDodgeInput(const FInputActionValue& InputActionValue);
	void HandleGuardStarted(const FInputActionValue& InputActionValue);
	void HandleGuardCompleted(const FInputActionValue& InputActionValue);
	void HandleSkillQInput(const FInputActionValue& InputActionValue);
	void HandleSkillEInput(const FInputActionValue& InputActionValue);
	void HandleMoveCompleted();
};
