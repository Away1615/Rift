// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/PlayerState.h"
#include "BasePlayerState.generated.h"

class URiftAbilitySystemComponent;
class URiftPlayerAttributeSet;
class URiftResourceAttributeSet;
class UPlayerClassConfig;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FRiftLobbyPlayerStateChangedSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FRiftRespawnStateChangedSignature);

/**
 *
 */
UCLASS()
class RIFT_API ABasePlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	ABasePlayerState();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void CopyProperties(APlayerState* PlayerState) override;

	UFUNCTION(BlueprintPure, Category="Rift|Lobby")
	UPlayerClassConfig* GetSelectedPlayerClassConfig() const { return SelectedPlayerClassConfig; }

	UFUNCTION(BlueprintPure, Category="Rift|Lobby")
	bool IsRoomHost() const { return bIsRoomHost; }

	UFUNCTION(BlueprintPure, Category="Rift|Respawn")
	bool IsWaitingForRespawn() const { return bIsWaitingForRespawn; }

	UFUNCTION(BlueprintPure, Category="Rift|Respawn")
	float GetRespawnEndServerTime() const { return RespawnEndServerTime; }

	UFUNCTION(BlueprintPure, Category="Rift|Respawn")
	float GetRespawnRemainingTime() const;

	void SetSelectedPlayerClassConfig(UPlayerClassConfig* NewPlayerClassConfig);
	void SetIsRoomHost(bool bNewIsRoomHost);
	void SetRespawnState(bool bWaiting, float EndServerTime, float Duration);

	UPROPERTY(BlueprintAssignable, Category="Rift|Lobby")
	FRiftLobbyPlayerStateChangedSignature OnLobbyPlayerStateChanged;

	UPROPERTY(BlueprintAssignable, Category="Rift|Respawn")
	FRiftRespawnStateChangedSignature OnRespawnStateChanged;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="AbilitySystem")
	TObjectPtr<URiftAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="AbilitySystem")
	TObjectPtr<URiftPlayerAttributeSet> AttributeSet;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="AbilitySystem")
	TObjectPtr<URiftResourceAttributeSet> ResourceAttributeSet;

	UPROPERTY(ReplicatedUsing=OnRep_SelectedPlayerClassConfig, BlueprintReadOnly, Category="Rift|Lobby")
	TObjectPtr<UPlayerClassConfig> SelectedPlayerClassConfig;

	UPROPERTY(ReplicatedUsing=OnRep_IsRoomHost, BlueprintReadOnly, Category="Rift|Lobby")
	bool bIsRoomHost = false;

	UPROPERTY(ReplicatedUsing=OnRep_RespawnState, BlueprintReadOnly, Category="Rift|Respawn")
	bool bIsWaitingForRespawn = false;

	UPROPERTY(ReplicatedUsing=OnRep_RespawnState, BlueprintReadOnly, Category="Rift|Respawn")
	float RespawnEndServerTime = 0.0f;

	UPROPERTY(ReplicatedUsing=OnRep_RespawnState, BlueprintReadOnly, Category="Rift|Respawn")
	float RespawnDuration = 0.0f;

	UFUNCTION()
	void OnRep_SelectedPlayerClassConfig();

	UFUNCTION()
	void OnRep_IsRoomHost();

	UFUNCTION()
	void OnRep_RespawnState();
};
