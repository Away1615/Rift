// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "RiftGameInstance.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FRiftRoomCodeSignature, const FString&, RoomCode);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FRiftRoomErrorSignature, const FString&, ErrorMessage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FRiftRoomSimpleSignature);

class UWorld;

UCLASS(Config=Game)
class RIFT_API URiftGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	URiftGameInstance();

	static const FName RoomCodeSessionKey;

	UFUNCTION(BlueprintCallable, Category="Rift|Session")
	void CreateLanRoom(const FString& RoomCode);

	UFUNCTION(BlueprintCallable, Category="Rift|Room")
	void JoinLanRoomByCode(const FString& RoomCode);

	UFUNCTION(BlueprintCallable, Category="Rift|Room")
	void LeaveRoom();

	UFUNCTION(BlueprintPure, Category="Rift|Room")
	FString GetCurrentRoomCode() const { return CurrentRoomCode; }

	UFUNCTION(BlueprintPure, Category="Rift|Room")
	bool IsInRoom() const;

	UFUNCTION(BlueprintPure, Category="Rift|Maps")
	FString GetLobbyMapPath() const;

	UFUNCTION(BlueprintPure, Category="Rift|Maps")
	FString GetGameplayMapPath() const;

	UFUNCTION(BlueprintPure, Category="Rift|Maps")
	bool HasValidLobbyMap() const;

	UFUNCTION(BlueprintPure, Category="Rift|Maps")
	bool HasValidGameplayMap() const;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Rift|Maps")
	TSoftObjectPtr<UWorld> LobbyMap;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Rift|Maps")
	TSoftObjectPtr<UWorld> GameplayMap;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Config, Category="Rift|Room", meta=(ClampMin="1", UIMin="1"))
	int32 MaxLobbyPlayers = 4;

	UPROPERTY(BlueprintAssignable, Category="Rift|Room")
	FRiftRoomCodeSignature OnCreateRoomSucceeded;

	UPROPERTY(BlueprintAssignable, Category="Rift|Room")
	FRiftRoomErrorSignature OnCreateRoomFailed;

	UPROPERTY(BlueprintAssignable, Category="Rift|Room")
	FRiftRoomCodeSignature OnJoinRoomSucceeded;

	UPROPERTY(BlueprintAssignable, Category="Rift|Room")
	FRiftRoomErrorSignature OnJoinRoomFailed;

	UPROPERTY(BlueprintAssignable, Category="Rift|Room")
	FRiftRoomErrorSignature OnFindRoomFailed;

	UPROPERTY(BlueprintAssignable, Category="Rift|Room")
	FRiftRoomSimpleSignature OnLeaveRoomCompleted;

private:
	IOnlineSessionPtr GetSessionInterface() const;
	FString NormalizeRoomCode(const FString& RoomCode) const;
	bool IsValidRoomCode(const FString& RoomCode) const;

	void HandleCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void HandleFindSessionsForCreateComplete(bool bWasSuccessful);
	void HandleFindSessionsComplete(bool bWasSuccessful);
	void HandleJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	void HandleDestroySessionComplete(FName SessionName, bool bWasSuccessful);

	void ClearCurrentRoom();
	void ClearCreateRoomCheck();
	void ClearJoinRoomSearch();
	void BroadcastJoinFailure(const FString& ErrorMessage);

	FString CurrentRoomCode;
	FString PendingCreateRoomCode;
	FString PendingJoinRoomCode;

	TSharedPtr<FOnlineSessionSettings> ActiveSessionSettings;
	TSharedPtr<FOnlineSessionSearch> ActiveCreateSessionSearch;
	TSharedPtr<FOnlineSessionSearch> ActiveSessionSearch;
	FOnlineSessionSearchResult PendingJoinSessionResult;

	FDelegateHandle CreateSessionCompleteDelegateHandle;
	FDelegateHandle FindSessionsForCreateCompleteDelegateHandle;
	FDelegateHandle FindSessionsCompleteDelegateHandle;
	FDelegateHandle JoinSessionCompleteDelegateHandle;
	FDelegateHandle DestroySessionCompleteDelegateHandle;
};
