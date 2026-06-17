// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "TimerManager.h"
#include "RiftLobbyGameMode.generated.h"

class ABasePlayerController;
class ABasePlayerState;
class UPlayerClassConfig;

UCLASS()
class RIFT_API ARiftLobbyGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ARiftLobbyGameMode();

	virtual void InitGameState() override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;

	UFUNCTION(BlueprintCallable, Category="Rift|Lobby")
	void StartGameFromLobby(ABasePlayerController* RequestingController);

	UPlayerClassConfig* GetDefaultPlayerClassConfig() const { return DefaultPlayerClassConfig; }
	void RefreshAllPlayersReady();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Rift|Lobby")
	float StartTransitionDuration = 3.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Rift|Lobby")
	TObjectPtr<UPlayerClassConfig> DefaultPlayerClassConfig;

private:
	void SyncRoomCodeToGameState() const;
	void SyncDefaultPlayerClassConfigToGameState() const;
	void BeginStartTransition();
	void NotifyPlayersStartTransition() const;
	void TravelToGameplayMap();
	void FailLobbyAction(ABasePlayerController* RequestingController, const FString& ErrorMessage) const;
	int32 FindAvailableLobbySlotIndex() const;
	void AssignLobbySlot(ABasePlayerState* PlayerState);
	void ReleaseLobbySlot(ABasePlayerState* PlayerState);
	void AssignFirstAvailableHost();
	bool HasRoomHost() const;
	bool AreAllPlayersConfirmed() const;
	bool AreAllPlayersReadyForGameplay() const;

	FTimerHandle StartGameTravelTimerHandle;
};
