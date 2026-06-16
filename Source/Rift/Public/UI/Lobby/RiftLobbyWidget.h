// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
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

	UFUNCTION(BlueprintCallable, Category="Rift|Lobby")
	void SelectPlayerClass(UPlayerClassConfig* ClassConfig);

	UFUNCTION(BlueprintCallable, Category="Rift|Lobby")
	void StartGame();

	UFUNCTION(BlueprintCallable, Category="Rift|Lobby")
	void RefreshLobbyState();

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION(BlueprintImplementableEvent, Category="Rift|Lobby")
	void OnLobbyStateRefreshed();

	UFUNCTION(BlueprintImplementableEvent, Category="Rift|Lobby")
	void OnLobbyStartTransition(float Duration);

	UFUNCTION(BlueprintImplementableEvent, Category="Rift|Lobby")
	void OnLobbyError(const FString& ErrorMessage);

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
