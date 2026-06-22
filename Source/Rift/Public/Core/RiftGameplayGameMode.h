// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/BaseGameMode.h"
#include "TimerManager.h"
#include "RiftGameplayGameMode.generated.h"

class ABasePlayerState;
class AEnemyCharacter;
class APlayerController;
class APlayerCharacter;

UCLASS()
class RIFT_API ARiftGameplayGameMode : public ABaseGameMode
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="Rift|Respawn")
	void NotifyPlayerDied(APlayerCharacter* DeadPlayer);

	UFUNCTION(BlueprintCallable, Category="Rift|Victory")
	void NotifyBossDefeated(AEnemyCharacter* DefeatedBoss);

	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Rift|Respawn")
	float PlayerRespawnDelay = 10.0f;

private:
	void ApplyCachedLobbySelection(APlayerController* PlayerController) const;
	void StartPlayerRespawn(APlayerCharacter* DeadPlayer);
	void FinishPlayerRespawn(ABasePlayerState* PlayerState);
	void DefeatRemainingEnemies(AEnemyCharacter* DefeatedBoss);
	void BroadcastVictoryToPlayers();

	TMap<ABasePlayerState*, FTimerHandle> RespawnTimers;
	bool bBossDefeated = false;
};
