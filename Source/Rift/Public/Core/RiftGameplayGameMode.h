// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/BaseGameMode.h"
#include "TimerManager.h"
#include "RiftGameplayGameMode.generated.h"

class ABasePlayerState;
class APlayerCharacter;

UCLASS()
class RIFT_API ARiftGameplayGameMode : public ABaseGameMode
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="Rift|Respawn")
	void NotifyPlayerDied(APlayerCharacter* DeadPlayer);

	virtual void Logout(AController* Exiting) override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Rift|Respawn")
	float PlayerRespawnDelay = 10.0f;

private:
	void StartPlayerRespawn(APlayerCharacter* DeadPlayer);
	void FinishPlayerRespawn(ABasePlayerState* PlayerState);

	TMap<ABasePlayerState*, FTimerHandle> RespawnTimers;
};
