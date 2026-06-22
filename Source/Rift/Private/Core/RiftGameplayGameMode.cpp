// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/RiftGameplayGameMode.h"

#include "Character/EnemyCharacter.h"
#include "Character/PlayerCharacter.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerController.h"
#include "Player/BasePlayerController.h"
#include "Player/BasePlayerState.h"

void ARiftGameplayGameMode::NotifyPlayerDied(APlayerCharacter* DeadPlayer)
{
	if (!HasAuthority() || !DeadPlayer)
	{
		return;
	}

	StartPlayerRespawn(DeadPlayer);
}

void ARiftGameplayGameMode::NotifyBossDefeated(AEnemyCharacter* DefeatedBoss)
{
	if (!HasAuthority() || bBossDefeated || !DefeatedBoss)
	{
		return;
	}

	bBossDefeated = true;
	DefeatRemainingEnemies(DefeatedBoss);
	BroadcastVictoryToPlayers();
}

void ARiftGameplayGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	ApplyCachedLobbySelection(NewPlayer);
}

void ARiftGameplayGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	ApplyCachedLobbySelection(NewPlayer);
	Super::HandleStartingNewPlayer_Implementation(NewPlayer);
}

void ARiftGameplayGameMode::Logout(AController* Exiting)
{
	ABasePlayerState* RiftPlayerState = Exiting ? Exiting->GetPlayerState<ABasePlayerState>() : nullptr;
	if (RiftPlayerState)
	{
		if (FTimerHandle* TimerHandle = RespawnTimers.Find(RiftPlayerState))
		{
			if (UWorld* World = GetWorld())
			{
				World->GetTimerManager().ClearTimer(*TimerHandle);
			}

			RespawnTimers.Remove(RiftPlayerState);
			RiftPlayerState->SetRespawnState(false, 0.0f, 0.0f);
		}
	}

	Super::Logout(Exiting);
}

void ARiftGameplayGameMode::ApplyCachedLobbySelection(APlayerController* PlayerController) const
{
	ABasePlayerController* RiftPlayerController = Cast<ABasePlayerController>(PlayerController);
	if (!RiftPlayerController)
	{
		return;
	}

	RiftPlayerController->ApplyCachedLobbySelectionToPlayerState();
}

void ARiftGameplayGameMode::StartPlayerRespawn(APlayerCharacter* DeadPlayer)
{
	if (!DeadPlayer)
	{
		return;
	}

	ABasePlayerState* RiftPlayerState = DeadPlayer->GetPlayerState<ABasePlayerState>();
	if (!RiftPlayerState)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	if (FTimerHandle* ExistingTimerHandle = RespawnTimers.Find(RiftPlayerState))
	{
		World->GetTimerManager().ClearTimer(*ExistingTimerHandle);
	}

	const float RespawnDelay = FMath::Max(0.0f, PlayerRespawnDelay);
	const AGameStateBase* CurrentGameState = GameState;
	const float CurrentServerTime = CurrentGameState
		? CurrentGameState->GetServerWorldTimeSeconds()
		: World->GetTimeSeconds();

	RiftPlayerState->SetRespawnState(true, CurrentServerTime + RespawnDelay, RespawnDelay);

	if (RespawnDelay <= 0.0f)
	{
		FinishPlayerRespawn(RiftPlayerState);
		return;
	}

	FTimerDelegate RespawnDelegate;
	RespawnDelegate.BindUObject(this, &ARiftGameplayGameMode::FinishPlayerRespawn, RiftPlayerState);

	FTimerHandle& TimerHandle = RespawnTimers.FindOrAdd(RiftPlayerState);
	World->GetTimerManager().SetTimer(
		TimerHandle,
		RespawnDelegate,
		RespawnDelay,
		false
	);
}

void ARiftGameplayGameMode::FinishPlayerRespawn(ABasePlayerState* PlayerState)
{
	if (!IsValid(PlayerState))
	{
		return;
	}

	RespawnTimers.Remove(PlayerState);

	UWorld* World = GetWorld();
	if (!World)
	{
		PlayerState->SetRespawnState(false, 0.0f, 0.0f);
		return;
	}

	APlayerController* PlayerController = nullptr;
	for (FConstPlayerControllerIterator Iterator = World->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		APlayerController* CandidateController = Iterator->Get();
		if (CandidateController && CandidateController->PlayerState == PlayerState)
		{
			PlayerController = CandidateController;
			break;
		}
	}

	APlayerCharacter* PlayerCharacter = PlayerController
		? PlayerController->GetPawn<APlayerCharacter>()
		: nullptr;

	if (!PlayerCharacter)
	{
		PlayerState->SetRespawnState(false, 0.0f, 0.0f);
		return;
	}

	PlayerCharacter->ReviveAtTransform(PlayerCharacter->GetActorTransform());
	PlayerState->SetRespawnState(false, 0.0f, 0.0f);
}

void ARiftGameplayGameMode::BroadcastVictoryToPlayers()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	for (FConstPlayerControllerIterator Iterator = World->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		if (ABasePlayerController* RiftPlayerController = Cast<ABasePlayerController>(Iterator->Get()))
		{
			RiftPlayerController->Client_ShowVictory();
		}
	}
}

void ARiftGameplayGameMode::DefeatRemainingEnemies(AEnemyCharacter* DefeatedBoss)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	for (TActorIterator<AEnemyCharacter> Iterator(World); Iterator; ++Iterator)
	{
		AEnemyCharacter* EnemyCharacter = *Iterator;
		if (!EnemyCharacter || EnemyCharacter == DefeatedBoss || EnemyCharacter->IsDeadForAI())
		{
			continue;
		}

		EnemyCharacter->HandleDeath(nullptr);
	}
}
