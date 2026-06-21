// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Appearance/RiftPlayerAppearanceTypes.h"
#include "Blueprint/UserWidget.h"
#include "Camera/PlayerCameraManager.h"
#include "TimerManager.h"
#include "RiftLobbyWidget.generated.h"

class AActor;
class ABasePlayerController;
class ABasePlayerState;
class ARiftLobbyDisplayActor;
class ARiftLobbyGameState;
class UListView;
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

	UFUNCTION(BlueprintCallable, Category="Rift|Lobby|Display")
	void RegisterLobbyDisplayActor(ARiftLobbyDisplayActor* DisplayActor);

	UFUNCTION(BlueprintCallable, Category="Rift|Lobby|Display")
	void RefreshLobbyDisplayActors();

	UFUNCTION(BlueprintPure, Category="Rift|Lobby|Display")
	ARiftLobbyDisplayActor* GetLobbyDisplayActorBySlotIndex(int32 SlotIndex) const;

	UFUNCTION(BlueprintCallable, Category="Lobby|Camera")
	bool SwitchToLocalSlotCamera();

	UFUNCTION(BlueprintCallable, Category="Lobby|Camera")
	bool SwitchToGroupCamera();

	UFUNCTION(BlueprintCallable, Category="Lobby|Camera")
	bool SwitchToCameraActor(AActor* CameraActor);

	UFUNCTION(BlueprintCallable, Category="Rift|Lobby|Display")
	void AddLocalPreviewYaw(float DeltaYaw);

	UFUNCTION(BlueprintCallable, Category="Rift|Lobby|Display")
	void SetLocalPreviewYaw(float NewPreviewYaw);

	UFUNCTION(BlueprintCallable, Category="Rift|Lobby|Display")
	void ResetLocalPreviewYaw();

	UFUNCTION(BlueprintCallable, Category="Rift|Lobby|Appearance")
	void RefreshAppearanceList(UListView* AppearanceListView, ERiftPlayerAppearanceSlot AppearanceSlot);

	UPROPERTY(BlueprintReadWrite, Category="Rift|Lobby|Appearance")
	FRiftPlayerAppearanceSelection LocalDraftAppearanceSelection;

	UPROPERTY(BlueprintReadWrite, Category="Rift|Lobby|Display")
	TArray<TObjectPtr<ARiftLobbyDisplayActor>> LobbyDisplayActors;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Lobby|Camera")
	FName GroupCameraTag = TEXT("Lobby_Group");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Lobby|Camera")
	float LobbyCameraBlendTime = 0.6f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Lobby|Camera")
	TEnumAsByte<EViewTargetBlendFunction> LobbyCameraBlendFunc = VTBlend_EaseInOut;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Lobby|Camera")
	float LobbyCameraBlendExp = 2.0f;

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
	void TryBindLobbyGameStateOrRetry();
	void StopLobbyGameStateBindRetry();
	void BindToLobbyGameState();
	void UnbindFromLobbyGameState();
	void BindToLobbyPlayerStates();
	void UnbindFromLobbyPlayerStates();
	void RegisterLobbyDisplayActorsInWorld();
	void UpdateLobbyCameraForLocalState();

	UFUNCTION()
	void HandleLobbyGameStateChanged();

	UFUNCTION()
	void HandleLobbyPlayerStateChanged();

	UFUNCTION()
	void HandleLobbyStartTransition(float Duration);

	UFUNCTION()
	void HandleLobbyActionFailed(const FString& ErrorMessage);

	ARiftLobbyDisplayActor* FindLobbyDisplayActorBySlotIndex(int32 SlotIndex) const;
	void RefreshLobbyDisplayActor(ARiftLobbyDisplayActor* DisplayActor);

	void RetryLocalSlotCameraSwitch();
	void StopLocalSlotCameraRetry();

	TWeakObjectPtr<ABasePlayerController> BoundPlayerController;
	TWeakObjectPtr<ARiftLobbyGameState> BoundLobbyGameState;
	TArray<TWeakObjectPtr<ABasePlayerState>> BoundPlayerStates;
	FTimerHandle LobbyGameStateBindRetryTimerHandle;
	FTimerHandle LocalSlotCameraRetryTimerHandle;
};
