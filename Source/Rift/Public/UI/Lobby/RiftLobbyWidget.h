// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Appearance/RiftPlayerAppearanceTypes.h"
#include "Blueprint/UserWidget.h"
#include "RiftLobbyWidget.generated.h"

class ABasePlayerController;
class ABasePlayerState;
class ARiftLobbyGameState;
class UPlayerClassConfig;

UCLASS()
class RIFT_API URiftLobbyWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="Rift|Lobby")
	FString GetRoomCode() const;

	UFUNCTION(BlueprintCallable, Category="Rift|Lobby")
	bool IsLocalPlayerRoomHost() const;

	UFUNCTION(BlueprintCallable, Category="Rift|Lobby")
	UPlayerClassConfig* GetSelectedPlayerClass() const;

	UFUNCTION(BlueprintPure, Category="Rift|Lobby")
	int32 GetLocalLobbySlotIndex() const;

	UFUNCTION(BlueprintPure, Category="Rift|Lobby")
	ABasePlayerState* GetPlayerStateByLobbySlotIndex(int32 SlotIndex) const;

	UFUNCTION(BlueprintPure, Category="Rift|Lobby")
	bool IsLocalLobbyCharacterConfirmed() const;

	UFUNCTION(BlueprintPure, Category="Rift|Lobby")
	bool IsPlayerStateLobbyCharacterConfirmed(ABasePlayerState* PlayerState) const;

	UFUNCTION(BlueprintCallable, Category="Rift|Lobby|Appearance")
	void SetLocalDraftAppearancePart(ERiftPlayerAppearanceSlot AppearanceSlot, FName PartId);

	UFUNCTION(BlueprintPure, Category="Rift|Lobby|Appearance")
	FRiftPlayerAppearanceSelection GetLocalDraftAppearanceSelection() const { return LocalDraftAppearanceSelection; }

	UFUNCTION(BlueprintCallable, Category="Rift|Lobby|Appearance")
	void ResetLocalDraftAppearance();

	UFUNCTION(BlueprintCallable, Category="Rift|Lobby")
	void RequestFinishLocalCharacterCreation();

	UFUNCTION(BlueprintPure, Category="Rift|Lobby")
	UPlayerClassConfig* GetLobbyDefaultPlayerClassConfig() const;

	UFUNCTION(BlueprintPure, Category="Rift|Lobby")
	bool AreAllPlayersReady() const;

	UFUNCTION(BlueprintCallable, Category="Rift|Lobby")
	void SelectPlayerClass(UPlayerClassConfig* ClassConfig);

	UFUNCTION(BlueprintCallable, Category="Rift|Lobby")
	void StartGame();

	UFUNCTION(BlueprintCallable, Category="Rift|Lobby")
	void RefreshLobbyState();

	UPROPERTY(BlueprintReadWrite, Category="Rift|Lobby|Appearance")
	FRiftPlayerAppearanceSelection LocalDraftAppearanceSelection;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION(BlueprintImplementableEvent, Category="Rift|Lobby")
	void OnLobbyStateRefreshed();

	UFUNCTION(BlueprintImplementableEvent, Category="Rift|Lobby")
	void OnLobbyStartTransition(float Duration);

	UFUNCTION(BlueprintImplementableEvent, Category="Rift|Lobby")
	void OnLobbyError(const FString& ErrorMessage);

	UFUNCTION(BlueprintImplementableEvent, Category="Rift|Lobby|Appearance")
	void OnLocalDraftAppearanceChanged(const FRiftPlayerAppearanceSelection& NewLocalDraftAppearanceSelection);

private:
	ABasePlayerController* GetRiftPlayerController() const;
	void BindToPlayerController();
	void UnbindFromPlayerController();
	void BindToLobbyGameState();
	void UnbindFromLobbyGameState();
	void BindToLobbyPlayerStates();
	void UnbindFromLobbyPlayerStates();

	UFUNCTION()
	void HandleLobbyGameStateChanged();

	UFUNCTION()
	void HandleLobbyPlayerStateChanged();

	UFUNCTION()
	void HandleLobbyStartTransition(float Duration);

	UFUNCTION()
	void HandleLobbyActionFailed(const FString& ErrorMessage);

	TWeakObjectPtr<ABasePlayerController> BoundPlayerController;
	TWeakObjectPtr<ARiftLobbyGameState> BoundLobbyGameState;
	TArray<TWeakObjectPtr<ABasePlayerState>> BoundPlayerStates;
};
