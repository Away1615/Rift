// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/BaseGameState.h"
#include "RiftLobbyGameState.generated.h"

class UPlayerClassConfig;

UENUM(BlueprintType)
enum class ERiftLobbyState : uint8
{
	Waiting UMETA(DisplayName="Waiting"),
	Starting UMETA(DisplayName="Starting")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FRiftLobbyGameStateChangedSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FRiftLobbyAllPlayersReadyChangedSignature, bool, bAllPlayersReady);

UCLASS()
class RIFT_API ARiftLobbyGameState : public ABaseGameState
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category="Rift|Lobby")
	FString GetRoomCode() const { return RoomCode; }

	UFUNCTION(BlueprintPure, Category="Rift|Lobby")
	ERiftLobbyState GetLobbyState() const { return LobbyState; }

	UFUNCTION(BlueprintPure, Category="Rift|Lobby")
	float GetStartTransitionDuration() const { return StartTransitionDuration; }

	UFUNCTION(BlueprintPure, Category="Rift|Lobby")
	bool IsStarting() const { return LobbyState == ERiftLobbyState::Starting; }

	UFUNCTION(BlueprintPure, Category="Rift|Lobby")
	UPlayerClassConfig* GetDefaultPlayerClassConfig() const { return DefaultPlayerClassConfig; }

	UFUNCTION(BlueprintPure, Category="Rift|Lobby")
	bool AreAllPlayersReady() const { return bAllPlayersReady; }

	void SetRoomCode(const FString& NewRoomCode);
	void SetLobbyState(ERiftLobbyState NewState);
	void SetStartTransitionDuration(float NewDuration);
	void SetDefaultPlayerClassConfig(UPlayerClassConfig* NewDefaultPlayerClassConfig);
	void SetAllPlayersReady(bool bNewAllPlayersReady);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void AddPlayerState(APlayerState* PlayerState) override;
	virtual void RemovePlayerState(APlayerState* PlayerState) override;

	UPROPERTY(BlueprintAssignable, Category="Rift|Lobby")
	FRiftLobbyGameStateChangedSignature OnLobbyGameStateChanged;

	UPROPERTY(BlueprintAssignable, Category="Rift|Lobby")
	FRiftLobbyAllPlayersReadyChangedSignature OnLobbyAllPlayersReadyChanged;

protected:
	UPROPERTY(ReplicatedUsing=OnRep_RoomCode, BlueprintReadOnly, Category="Rift|Lobby")
	FString RoomCode;

	UPROPERTY(ReplicatedUsing=OnRep_LobbyState, BlueprintReadOnly, Category="Rift|Lobby")
	ERiftLobbyState LobbyState = ERiftLobbyState::Waiting;

	UPROPERTY(ReplicatedUsing=OnRep_StartTransitionDuration, BlueprintReadOnly, Category="Rift|Lobby")
	float StartTransitionDuration = 0.0f;

	UPROPERTY(ReplicatedUsing=OnRep_DefaultPlayerClassConfig, BlueprintReadOnly, Category="Rift|Lobby")
	TObjectPtr<UPlayerClassConfig> DefaultPlayerClassConfig;

	UPROPERTY(ReplicatedUsing=OnRep_AllPlayersReady, BlueprintReadOnly, Category="Rift|Lobby")
	bool bAllPlayersReady = false;

	UFUNCTION()
	void OnRep_RoomCode();

	UFUNCTION()
	void OnRep_LobbyState();

	UFUNCTION()
	void OnRep_StartTransitionDuration();

	UFUNCTION()
	void OnRep_DefaultPlayerClassConfig();

	UFUNCTION()
	void OnRep_AllPlayersReady();
};
