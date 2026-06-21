// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/RiftGameInstance.h"

#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "OnlineSubsystem.h"
#include "UObject/UObjectGlobals.h"

const FName URiftGameInstance::RoomCodeSessionKey(TEXT("ROOM_CODE"));

DEFINE_LOG_CATEGORY_STATIC(LogRiftGameInstance, Log, All);

URiftGameInstance::URiftGameInstance()
{
}

void URiftGameInstance::CreateLanRoom(const FString& RoomCode)
{
	if (ActiveCreateSessionSearch.IsValid() ||
		FindSessionsForCreateCompleteDelegateHandle.IsValid() ||
		PostLoadMapWithWorldDelegateHandle.IsValid() ||
		CreateSessionCompleteDelegateHandle.IsValid())
	{
		OnCreateRoomFailed.Broadcast(TEXT("Room creation is already in progress."));
		return;
	}

	PendingCreateRoomCode = NormalizeRoomCode(RoomCode);
	if (!IsValidRoomCode(PendingCreateRoomCode))
	{
		ClearCreateRoomCheck();
		OnCreateRoomFailed.Broadcast(TEXT("Room code must be 4-12 letters or numbers."));
		return;
	}

	if (!HasValidLobbyMap())
	{
		ClearCreateRoomCheck();
		OnCreateRoomFailed.Broadcast(TEXT("Lobby map is not configured."));
		return;
	}

	IOnlineSessionPtr SessionInterface = GetSessionInterface();
	if (!SessionInterface.IsValid())
	{
		ClearCreateRoomCheck();
		OnCreateRoomFailed.Broadcast(TEXT("Online session interface is unavailable."));
		return;
	}

	if (!CurrentRoomCode.IsEmpty() || SessionInterface->GetNamedSession(NAME_GameSession))
	{
		ClearCreateRoomCheck();
		OnCreateRoomFailed.Broadcast(TEXT("Already in a room."));
		return;
	}

	ActiveCreateSessionSearch = MakeShared<FOnlineSessionSearch>();
	ActiveCreateSessionSearch->bIsLanQuery = true;
	ActiveCreateSessionSearch->MaxSearchResults = 50;

	FindSessionsForCreateCompleteDelegateHandle = SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(
		FOnFindSessionsCompleteDelegate::CreateUObject(
			this,
			&URiftGameInstance::HandleFindSessionsForCreateComplete
		)
	);

	if (!SessionInterface->FindSessions(0, ActiveCreateSessionSearch.ToSharedRef()))
	{
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsForCreateCompleteDelegateHandle);
		ClearCreateRoomCheck();
		OnCreateRoomFailed.Broadcast(TEXT("Failed to start room code check."));
	}
}

void URiftGameInstance::JoinLanRoomByCode(const FString& RoomCode)
{
	if (ActiveSessionSearch.IsValid() ||
		FindSessionsCompleteDelegateHandle.IsValid() ||
		JoinSessionCompleteDelegateHandle.IsValid())
	{
		BroadcastJoinFailure(TEXT("Already searching room."));
		return;
	}

	PendingJoinRoomCode = NormalizeRoomCode(RoomCode);

	if (!IsValidRoomCode(PendingJoinRoomCode))
	{
		PendingJoinRoomCode.Empty();
		BroadcastJoinFailure(TEXT("Room code must be 4-12 letters or numbers."));
		return;
	}

	IOnlineSessionPtr SessionInterface = GetSessionInterface();
	if (!SessionInterface.IsValid())
	{
		BroadcastJoinFailure(TEXT("Online session interface is unavailable."));
		return;
	}

	if (SessionInterface->GetNamedSession(NAME_GameSession))
	{
		BroadcastJoinFailure(TEXT("Already in a room."));
		return;
	}

	ActiveSessionSearch = MakeShared<FOnlineSessionSearch>();
	ActiveSessionSearch->bIsLanQuery = true;
	ActiveSessionSearch->MaxSearchResults = 50;

	FindSessionsCompleteDelegateHandle = SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(
		FOnFindSessionsCompleteDelegate::CreateUObject(this, &URiftGameInstance::HandleFindSessionsComplete)
	);

	if (!SessionInterface->FindSessions(0, ActiveSessionSearch.ToSharedRef()))
	{
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
		ClearJoinRoomSearch();
		BroadcastJoinFailure(TEXT("Failed to start room search."));
	}
}

void URiftGameInstance::LeaveRoom()
{
	IOnlineSessionPtr SessionInterface = GetSessionInterface();
	ClearCurrentRoom();

	if (!SessionInterface.IsValid() || !SessionInterface->GetNamedSession(NAME_GameSession))
	{
		OnLeaveRoomCompleted.Broadcast();
		return;
	}

	DestroySessionCompleteDelegateHandle = SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(
		FOnDestroySessionCompleteDelegate::CreateUObject(this, &URiftGameInstance::HandleDestroySessionComplete)
	);

	if (!SessionInterface->DestroySession(NAME_GameSession))
	{
		SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
		DestroySessionCompleteDelegateHandle.Reset();
		OnLeaveRoomCompleted.Broadcast();
	}
}

bool URiftGameInstance::IsInRoom() const
{
	const IOnlineSessionPtr SessionInterface = GetSessionInterface();
	return !CurrentRoomCode.IsEmpty() || (SessionInterface.IsValid() && SessionInterface->GetNamedSession(NAME_GameSession));
}

FString URiftGameInstance::GetLobbyMapPath() const
{
	return LobbyMap.IsNull() ? FString() : LobbyMap.ToSoftObjectPath().GetLongPackageName();
}

FString URiftGameInstance::GetGameplayMapPath() const
{
	return GameplayMap.IsNull() ? FString() : GameplayMap.ToSoftObjectPath().GetLongPackageName();
}

bool URiftGameInstance::HasValidLobbyMap() const
{
	return !GetLobbyMapPath().IsEmpty();
}

bool URiftGameInstance::HasValidGameplayMap() const
{
	return !GetGameplayMapPath().IsEmpty();
}

IOnlineSessionPtr URiftGameInstance::GetSessionInterface() const
{
	IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
	return OnlineSubsystem ? OnlineSubsystem->GetSessionInterface() : nullptr;
}

FString URiftGameInstance::NormalizeRoomCode(const FString& RoomCode) const
{
	FString NormalizedRoomCode = RoomCode;
	NormalizedRoomCode.TrimStartAndEndInline();
	NormalizedRoomCode.ToUpperInline();

	return NormalizedRoomCode;
}

bool URiftGameInstance::IsValidRoomCode(const FString& RoomCode) const
{
	if (RoomCode.Len() < 4 || RoomCode.Len() > 12)
	{
		return false;
	}

	for (const TCHAR Character : RoomCode)
	{
		const bool bIsDigit = Character >= TEXT('0') && Character <= TEXT('9');
		const bool bIsUpperLetter = Character >= TEXT('A') && Character <= TEXT('Z');
		if (!bIsDigit && !bIsUpperLetter)
		{
			return false;
		}
	}

	return true;
}

void URiftGameInstance::HandleFindSessionsForCreateComplete(bool bWasSuccessful)
{
	IOnlineSessionPtr SessionInterface = GetSessionInterface();
	if (SessionInterface.IsValid())
	{
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsForCreateCompleteDelegateHandle);
	}
	FindSessionsForCreateCompleteDelegateHandle.Reset();

	if (!SessionInterface.IsValid())
	{
		ClearCreateRoomCheck();
		OnCreateRoomFailed.Broadcast(TEXT("Online session interface is unavailable."));
		return;
	}

	if (!bWasSuccessful || !ActiveCreateSessionSearch.IsValid())
	{
		ClearCreateRoomCheck();
		OnCreateRoomFailed.Broadcast(TEXT("Failed to check existing LAN rooms."));
		return;
	}

	for (const FOnlineSessionSearchResult& SearchResult : ActiveCreateSessionSearch->SearchResults)
	{
		FString FoundRoomCode;
		if (!SearchResult.Session.SessionSettings.Get(RoomCodeSessionKey, FoundRoomCode))
		{
			continue;
		}

		if (NormalizeRoomCode(FoundRoomCode) == PendingCreateRoomCode)
		{
			ClearCreateRoomCheck();
			OnCreateRoomFailed.Broadcast(TEXT("Room code already exists."));
			return;
		}
	}

	ActiveCreateSessionSearch.Reset();
	CurrentRoomCode = PendingCreateRoomCode;
	PendingCreateRoomCode.Empty();

	ClearLobbyMapLoadDelegate();
	PostLoadMapWithWorldDelegateHandle = FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(
		this,
		&URiftGameInstance::HandleLobbyMapLoaded
	);

	const FString LobbyMapPath = GetLobbyMapPath();
	UGameplayStatics::OpenLevel(this, FName(*LobbyMapPath), true, TEXT("listen"));
}

void URiftGameInstance::StartCreateSessionForCurrentRoom()
{
	IOnlineSessionPtr SessionInterface = GetSessionInterface();
	if (!SessionInterface.IsValid())
	{
		ClearCurrentRoom();
		OnCreateRoomFailed.Broadcast(TEXT("Online session interface is unavailable."));
		return;
	}

	if (CurrentRoomCode.IsEmpty())
	{
		ClearCurrentRoom();
		OnCreateRoomFailed.Broadcast(TEXT("Room code is missing."));
		return;
	}

	if (SessionInterface->GetNamedSession(NAME_GameSession))
	{
		ClearCurrentRoom();
		OnCreateRoomFailed.Broadcast(TEXT("Already in a room."));
		return;
	}

	ActiveSessionSettings = MakeShared<FOnlineSessionSettings>();
	ActiveSessionSettings->bIsLANMatch = true;
	ActiveSessionSettings->bShouldAdvertise = true;
	ActiveSessionSettings->bAllowJoinInProgress = true;
	ActiveSessionSettings->NumPublicConnections = MaxLobbyPlayers;
	ActiveSessionSettings->Set(
		RoomCodeSessionKey,
		CurrentRoomCode,
		EOnlineDataAdvertisementType::ViaOnlineServiceAndPing
	);

	CreateSessionCompleteDelegateHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(
		FOnCreateSessionCompleteDelegate::CreateUObject(this, &URiftGameInstance::HandleCreateSessionComplete)
	);

	if (!SessionInterface->CreateSession(0, NAME_GameSession, *ActiveSessionSettings))
	{
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
		CreateSessionCompleteDelegateHandle.Reset();
		ClearCurrentRoom();
		OnCreateRoomFailed.Broadcast(TEXT("Failed to start session creation."));
	}
}

void URiftGameInstance::HandleLobbyMapLoaded(UWorld* LoadedWorld)
{
	ClearLobbyMapLoadDelegate();

	if (!LoadedWorld)
	{
		ClearCurrentRoom();
		OnCreateRoomFailed.Broadcast(TEXT("Failed to load lobby map."));
		return;
	}

	UE_LOG(LogRiftGameInstance, Log, TEXT("Lobby map loaded. Creating LAN session for room %s."), *CurrentRoomCode);
	StartCreateSessionForCurrentRoom();
}

void URiftGameInstance::HandleCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	IOnlineSessionPtr SessionInterface = GetSessionInterface();
	if (SessionInterface.IsValid())
	{
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
	}
	CreateSessionCompleteDelegateHandle.Reset();

	if (SessionName != NAME_GameSession)
	{
		return;
	}

	if (!bWasSuccessful)
	{
		ClearCurrentRoom();
		OnCreateRoomFailed.Broadcast(TEXT("Failed to create LAN room."));
		return;
	}

	OnCreateRoomSucceeded.Broadcast(CurrentRoomCode);
}

void URiftGameInstance::HandleFindSessionsComplete(bool bWasSuccessful)
{
	IOnlineSessionPtr SessionInterface = GetSessionInterface();
	if (SessionInterface.IsValid())
	{
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
	}
	FindSessionsCompleteDelegateHandle.Reset();

	if (!SessionInterface.IsValid())
	{
		ClearJoinRoomSearch();
		BroadcastJoinFailure(TEXT("Online session interface is unavailable."));
		return;
	}

	if (!bWasSuccessful || !ActiveSessionSearch.IsValid())
	{
		ClearJoinRoomSearch();
		OnFindRoomFailed.Broadcast(TEXT("Failed to find LAN rooms."));
		return;
	}

	for (const FOnlineSessionSearchResult& SearchResult : ActiveSessionSearch->SearchResults)
	{
		FString FoundRoomCode;
		if (!SearchResult.Session.SessionSettings.Get(RoomCodeSessionKey, FoundRoomCode))
		{
			continue;
		}

		FoundRoomCode.TrimStartAndEndInline();
		FoundRoomCode.ToUpperInline();

		if (FoundRoomCode != PendingJoinRoomCode)
		{
			continue;
		}

		PendingJoinSessionResult = SearchResult;
		ActiveSessionSearch.Reset();
		JoinSessionCompleteDelegateHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(
			FOnJoinSessionCompleteDelegate::CreateUObject(this, &URiftGameInstance::HandleJoinSessionComplete)
		);

		if (!SessionInterface->JoinSession(0, NAME_GameSession, PendingJoinSessionResult))
		{
			SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
			ClearJoinRoomSearch();
			BroadcastJoinFailure(TEXT("Failed to start joining LAN room."));
		}
		return;
	}

	ClearJoinRoomSearch();
	OnFindRoomFailed.Broadcast(TEXT("No LAN room matched the room code."));
}

void URiftGameInstance::HandleJoinSessionComplete(
	FName SessionName,
	EOnJoinSessionCompleteResult::Type Result
)
{
	IOnlineSessionPtr SessionInterface = GetSessionInterface();
	if (SessionInterface.IsValid())
	{
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
	}
	JoinSessionCompleteDelegateHandle.Reset();

	if (SessionName != NAME_GameSession)
	{
		ClearJoinRoomSearch();
		return;
	}

	if (Result != EOnJoinSessionCompleteResult::Success || !SessionInterface.IsValid())
	{
		ClearJoinRoomSearch();
		BroadcastJoinFailure(TEXT("Failed to join LAN room."));
		return;
	}

	FString ConnectString;
	if (!SessionInterface->GetResolvedConnectString(NAME_GameSession, ConnectString))
	{
		ClearJoinRoomSearch();
		BroadcastJoinFailure(TEXT("Failed to resolve host address."));
		return;
	}

	if (ConnectString.Contains(TEXT(":0/")) || ConnectString.EndsWith(TEXT(":0")))
	{
		UE_LOG(LogRiftGameInstance, Warning, TEXT("Resolved invalid room connect string: %s"), *ConnectString);
		ClearJoinRoomSearch();
		BroadcastJoinFailure(TEXT("Resolved host address is invalid."));
		return;
	}

	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	if (!PlayerController)
	{
		ClearJoinRoomSearch();
		BroadcastJoinFailure(TEXT("Failed to get local player controller."));
		return;
	}

	CurrentRoomCode = PendingJoinRoomCode;
	OnJoinRoomSucceeded.Broadcast(CurrentRoomCode);
	PlayerController->ClientTravel(ConnectString, TRAVEL_Absolute);
	ClearJoinRoomSearch();
}

void URiftGameInstance::HandleDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	static_cast<void>(bWasSuccessful);

	IOnlineSessionPtr SessionInterface = GetSessionInterface();
	if (SessionInterface.IsValid())
	{
		SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
	}
	DestroySessionCompleteDelegateHandle.Reset();

	if (SessionName == NAME_GameSession)
	{
		OnLeaveRoomCompleted.Broadcast();
	}
}

void URiftGameInstance::ClearCurrentRoom()
{
	CurrentRoomCode.Empty();
	ClearCreateRoomCheck();
	ClearJoinRoomSearch();
}

void URiftGameInstance::ClearLobbyMapLoadDelegate()
{
	if (PostLoadMapWithWorldDelegateHandle.IsValid())
	{
		FCoreUObjectDelegates::PostLoadMapWithWorld.Remove(PostLoadMapWithWorldDelegateHandle);
		PostLoadMapWithWorldDelegateHandle.Reset();
	}
}

void URiftGameInstance::ClearCreateRoomCheck()
{
	IOnlineSessionPtr SessionInterface = GetSessionInterface();
	if (SessionInterface.IsValid() && FindSessionsForCreateCompleteDelegateHandle.IsValid())
	{
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsForCreateCompleteDelegateHandle);
	}

	PendingCreateRoomCode.Empty();
	ActiveCreateSessionSearch.Reset();
	ClearLobbyMapLoadDelegate();
	FindSessionsForCreateCompleteDelegateHandle.Reset();
}

void URiftGameInstance::ClearJoinRoomSearch()
{
	IOnlineSessionPtr SessionInterface = GetSessionInterface();
	if (SessionInterface.IsValid())
	{
		if (FindSessionsCompleteDelegateHandle.IsValid())
		{
			SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
		}

		if (JoinSessionCompleteDelegateHandle.IsValid())
		{
			SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
		}
	}

	PendingJoinRoomCode.Empty();
	ActiveSessionSearch.Reset();
	PendingJoinSessionResult = FOnlineSessionSearchResult();
	FindSessionsCompleteDelegateHandle.Reset();
	JoinSessionCompleteDelegateHandle.Reset();
}

void URiftGameInstance::BroadcastJoinFailure(const FString& ErrorMessage)
{
	OnJoinRoomFailed.Broadcast(ErrorMessage);
}
