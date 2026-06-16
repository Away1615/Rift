// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "TimerManager.h"
#include "RiftLobbyGameMode.generated.h"

class ABasePlayerController;

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

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Rift|Lobby")
	float StartTransitionDuration = 3.0f;

private:
	void SyncRoomCodeToGameState() const;
	void BeginStartTransition();
	void NotifyPlayersStartTransition() const;
	void TravelToGameplayMap();
	void FailLobbyAction(ABasePlayerController* RequestingController, const FString& ErrorMessage) const;
	void AssignFirstAvailableHost();
	bool HasRoomHost() const;
	bool AreAllPlayersReadyForGameplay() const;

	FTimerHandle StartGameTravelTimerHandle;
};
