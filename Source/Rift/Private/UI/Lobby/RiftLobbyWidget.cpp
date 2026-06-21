// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Lobby/RiftLobbyWidget.h"

#include "Core/RiftLobbyGameState.h"
#include "Components/ListView.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "Player/BasePlayerController.h"
#include "Player/BasePlayerState.h"
#include "UI/Lobby/RiftAppearanceOptionItemObject.h"
#include "UI/Lobby/RiftLobbyDisplayActor.h"

namespace
{
	constexpr int32 MaxLobbyDisplaySlots = 4;
}

void URiftLobbyWidget::NativeConstruct()
{
	Super::NativeConstruct();

	BindToPlayerController();
	RegisterLobbyDisplayActorsInWorld();
	TryBindLobbyGameStateOrRetry();
	UpdateLobbyCameraForLocalState();
}

void URiftLobbyWidget::NativeDestruct()
{
	StopLobbyGameStateBindRetry();
	StopLocalSlotCameraRetry();
	UnbindFromLobbyPlayerStates();
	UnbindFromLobbyGameState();
	UnbindFromPlayerController();

	Super::NativeDestruct();
}

FString URiftLobbyWidget::GetRoomCode() const
{
	const UWorld* World = GetWorld();
	const ARiftLobbyGameState* LobbyGameState = World ? World->GetGameState<ARiftLobbyGameState>() : nullptr;
	return LobbyGameState ? LobbyGameState->GetRoomCode() : FString();
}

bool URiftLobbyWidget::IsLocalPlayerRoomHost() const
{
	const ABasePlayerController* RiftPlayerController = GetRiftPlayerController();
	const ABasePlayerState* RiftPlayerState = RiftPlayerController
		? RiftPlayerController->GetPlayerState<ABasePlayerState>()
		: nullptr;

	return RiftPlayerState && RiftPlayerState->IsRoomHost();
}

UPlayerClassConfig* URiftLobbyWidget::GetSelectedPlayerClass() const
{
	const ABasePlayerController* RiftPlayerController = GetRiftPlayerController();
	const ABasePlayerState* RiftPlayerState = RiftPlayerController
		? RiftPlayerController->GetPlayerState<ABasePlayerState>()
		: nullptr;

	return RiftPlayerState ? RiftPlayerState->GetSelectedPlayerClassConfig() : nullptr;
}

int32 URiftLobbyWidget::GetLocalLobbySlotIndex() const
{
	const ABasePlayerController* RiftPlayerController = GetRiftPlayerController();
	const ABasePlayerState* RiftPlayerState = RiftPlayerController
		? RiftPlayerController->GetPlayerState<ABasePlayerState>()
		: nullptr;

	return RiftPlayerState ? RiftPlayerState->GetLobbySlotIndex() : INDEX_NONE;
}

ABasePlayerState* URiftLobbyWidget::GetPlayerStateByLobbySlotIndex(const int32 SlotIndex) const
{
	if (SlotIndex < 0 || SlotIndex >= MaxLobbyDisplaySlots)
	{
		return nullptr;
	}

	ABasePlayerController* RiftPlayerController = GetRiftPlayerController();
	ABasePlayerState* LocalPlayerState = RiftPlayerController
		? RiftPlayerController->GetPlayerState<ABasePlayerState>()
		: nullptr;
	if (LocalPlayerState && LocalPlayerState->GetLobbySlotIndex() == SlotIndex)
	{
		return LocalPlayerState;
	}

	const UWorld* World = GetWorld();
	const ARiftLobbyGameState* LobbyGameState = World ? World->GetGameState<ARiftLobbyGameState>() : nullptr;
	if (!LobbyGameState)
	{
		return nullptr;
	}

	for (APlayerState* PlayerState : LobbyGameState->PlayerArray)
	{
		ABasePlayerState* RiftPlayerState = Cast<ABasePlayerState>(PlayerState);
		if (RiftPlayerState && RiftPlayerState->GetLobbySlotIndex() == SlotIndex)
		{
			return RiftPlayerState;
		}
	}

	return nullptr;
}

bool URiftLobbyWidget::IsLocalLobbyCharacterConfirmed() const
{
	const ABasePlayerController* RiftPlayerController = GetRiftPlayerController();
	const ABasePlayerState* RiftPlayerState = RiftPlayerController
		? RiftPlayerController->GetPlayerState<ABasePlayerState>()
		: nullptr;

	return RiftPlayerState && RiftPlayerState->IsLobbyCharacterConfirmed();
}

bool URiftLobbyWidget::IsPlayerStateLobbyCharacterConfirmed(ABasePlayerState* PlayerState) const
{
	return PlayerState && PlayerState->IsLobbyCharacterConfirmed();
}

void URiftLobbyWidget::SetLocalDraftAppearancePart(const ERiftPlayerAppearanceSlot AppearanceSlot, const FName PartId)
{
	if (LocalDraftAppearanceSelection.GetPartId(AppearanceSlot) == PartId)
	{
		return;
	}

	LocalDraftAppearanceSelection.SetPartId(AppearanceSlot, PartId);
	OnLocalDraftAppearanceChanged(LocalDraftAppearanceSelection);
	RefreshLobbyDisplayActors();
}

void URiftLobbyWidget::ResetLocalDraftAppearance()
{
	LocalDraftAppearanceSelection = FRiftPlayerAppearanceSelection();
	OnLocalDraftAppearanceChanged(LocalDraftAppearanceSelection);
	RefreshLobbyDisplayActors();
}

void URiftLobbyWidget::RequestFinishLocalCharacterCreation()
{
	ABasePlayerController* RiftPlayerController = GetRiftPlayerController();
	if (!RiftPlayerController)
	{
		return;
	}

	RiftPlayerController->RequestFinishCharacterCreationWithAppearance(LocalDraftAppearanceSelection);
}

UPlayerClassConfig* URiftLobbyWidget::GetLobbyDefaultPlayerClassConfig() const
{
	const UWorld* World = GetWorld();
	const ARiftLobbyGameState* LobbyGameState = World ? World->GetGameState<ARiftLobbyGameState>() : nullptr;
	return LobbyGameState ? LobbyGameState->GetDefaultPlayerClassConfig() : nullptr;
}

bool URiftLobbyWidget::AreAllPlayersReady() const
{
	const UWorld* World = GetWorld();
	const ARiftLobbyGameState* LobbyGameState = World ? World->GetGameState<ARiftLobbyGameState>() : nullptr;
	return LobbyGameState && LobbyGameState->AreAllPlayersReady();
}

void URiftLobbyWidget::SelectPlayerClass(UPlayerClassConfig* ClassConfig)
{
	if (!ClassConfig)
	{
		return;
	}

	ABasePlayerController* RiftPlayerController = GetRiftPlayerController();
	if (!RiftPlayerController)
	{
		return;
	}

	RiftPlayerController->RequestSelectPlayerClass(ClassConfig);
	RefreshLobbyState();
}

void URiftLobbyWidget::StartGame()
{
	ABasePlayerController* RiftPlayerController = GetRiftPlayerController();
	if (!RiftPlayerController)
	{
		return;
	}

	RiftPlayerController->RequestStartLobbyGame();
}

void URiftLobbyWidget::RefreshLobbyState()
{
	BindToLobbyGameState();
	UnbindFromLobbyPlayerStates();
	BindToLobbyPlayerStates();

	RefreshLobbyDisplayActors();
	UpdateLobbyCameraForLocalState();
	OnLobbyStateRefreshed();
}

void URiftLobbyWidget::RegisterLobbyDisplayActor(ARiftLobbyDisplayActor* DisplayActor)
{
	if (!DisplayActor)
	{
		return;
	}

	LobbyDisplayActors.AddUnique(DisplayActor);
	RefreshLobbyDisplayActor(DisplayActor);
}

void URiftLobbyWidget::RefreshLobbyDisplayActors()
{
	for (int32 Index = LobbyDisplayActors.Num() - 1; Index >= 0; --Index)
	{
		ARiftLobbyDisplayActor* DisplayActor = LobbyDisplayActors[Index];
		if (!IsValid(DisplayActor))
		{
			LobbyDisplayActors.RemoveAt(Index);
			continue;
		}

		RefreshLobbyDisplayActor(DisplayActor);
	}
}

ARiftLobbyDisplayActor* URiftLobbyWidget::GetLobbyDisplayActorBySlotIndex(const int32 SlotIndex) const
{
	return FindLobbyDisplayActorBySlotIndex(SlotIndex);
}

bool URiftLobbyWidget::SwitchToLocalSlotCamera()
{
	if (IsLocalLobbyCharacterConfirmed())
	{
		StopLocalSlotCameraRetry();
		return false;
	}

	const int32 LocalSlotIndex = GetLocalLobbySlotIndex();
	const ARiftLobbyDisplayActor* DisplayActor = LocalSlotIndex >= 0
		? FindLobbyDisplayActorBySlotIndex(LocalSlotIndex)
		: nullptr;

	if (IsValid(DisplayActor) && SwitchToCameraActor(DisplayActor->GetSlotCameraActor()))
	{
		StopLocalSlotCameraRetry();
		RefreshLobbyDisplayActors();
		return true;
	}

	// LocalSlotIndex / the matching display actor may not have replicated yet
	// (e.g. right after a second player joins). Keep retrying until it has,
	// instead of leaving the view target unset (camera defaults to the origin
	// since the lobby has no Pawn).
	UWorld* World = GetWorld();
	if (World && !World->GetTimerManager().IsTimerActive(LocalSlotCameraRetryTimerHandle))
	{
		World->GetTimerManager().SetTimer(
			LocalSlotCameraRetryTimerHandle,
			this,
			&URiftLobbyWidget::RetryLocalSlotCameraSwitch,
			0.15f,
			true
		);
	}

	return false;
}

bool URiftLobbyWidget::SwitchToGroupCamera()
{
	if (GroupCameraTag.IsNone())
	{
		return false;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	TArray<AActor*> CameraActors;
	UGameplayStatics::GetAllActorsWithTag(World, GroupCameraTag, CameraActors);

	for (AActor* CameraActor : CameraActors)
	{
		if (IsValid(CameraActor))
		{
			return SwitchToCameraActor(CameraActor);
		}
	}

	return false;
}

bool URiftLobbyWidget::SwitchToCameraActor(AActor* CameraActor)
{
	APlayerController* PlayerController = GetOwningPlayer();
	if (!PlayerController || !IsValid(CameraActor))
	{
		return false;
	}

	PlayerController->SetViewTargetWithBlend(
		CameraActor,
		LobbyCameraBlendTime,
		LobbyCameraBlendFunc,
		LobbyCameraBlendExp
	);
	return true;
}

ARiftLobbyDisplayActor* URiftLobbyWidget::FindLobbyDisplayActorBySlotIndex(const int32 SlotIndex) const
{
	for (ARiftLobbyDisplayActor* DisplayActor : LobbyDisplayActors)
	{
		if (IsValid(DisplayActor) && DisplayActor->GetLobbySlotIndex() == SlotIndex)
		{
			return DisplayActor;
		}
	}

	return nullptr;
}

void URiftLobbyWidget::AddLocalPreviewYaw(const float DeltaYaw)
{
	ARiftLobbyDisplayActor* DisplayActor = GetLobbyDisplayActorBySlotIndex(GetLocalLobbySlotIndex());
	if (!DisplayActor)
	{
		return;
	}

	DisplayActor->AddPreviewYaw(DeltaYaw);
}

void URiftLobbyWidget::SetLocalPreviewYaw(const float NewPreviewYaw)
{
	ARiftLobbyDisplayActor* DisplayActor = GetLobbyDisplayActorBySlotIndex(GetLocalLobbySlotIndex());
	if (!DisplayActor)
	{
		return;
	}

	DisplayActor->SetPreviewYaw(NewPreviewYaw);
}

void URiftLobbyWidget::ResetLocalPreviewYaw()
{
	ARiftLobbyDisplayActor* DisplayActor = GetLobbyDisplayActorBySlotIndex(GetLocalLobbySlotIndex());
	if (!DisplayActor)
	{
		return;
	}

	DisplayActor->ResetPreviewYaw();
}

void URiftLobbyWidget::RefreshAppearanceList(
	UListView* AppearanceListView,
	const ERiftPlayerAppearanceSlot AppearanceSlot
)
{
	if (!AppearanceListView)
	{
		return;
	}

	AppearanceListView->ClearListItems();

	const ABasePlayerController* RiftPlayerController = GetRiftPlayerController();
	if (!RiftPlayerController)
	{
		return;
	}

	TArray<FName> PartIds;
	RiftPlayerController->GetAppearanceOptionsForSlot(AppearanceSlot, PartIds);

	for (const FName& PartId : PartIds)
	{
		URiftAppearanceOptionItemObject* OptionItem =
			NewObject<URiftAppearanceOptionItemObject>(this);
		if (!OptionItem)
		{
			continue;
		}

		FText DisplayName = RiftPlayerController->GetAppearancePartDisplayName(PartId);
		if (DisplayName.IsEmpty())
		{
			DisplayName = FText::FromString(PartId.ToString());
		}

		OptionItem->InitializeAppearanceOption(this, AppearanceSlot, PartId, DisplayName);
		AppearanceListView->AddItem(OptionItem);
	}
}

ABasePlayerController* URiftLobbyWidget::GetRiftPlayerController() const
{
	return Cast<ABasePlayerController>(GetOwningPlayer());
}

void URiftLobbyWidget::BindToPlayerController()
{
	ABasePlayerController* RiftPlayerController = GetRiftPlayerController();
	if (!RiftPlayerController)
	{
		return;
	}

	BoundPlayerController = RiftPlayerController;
	RiftPlayerController->OnLobbyStartTransitionRequested.AddUniqueDynamic(
		this,
		&URiftLobbyWidget::HandleLobbyStartTransition
	);
	RiftPlayerController->OnLobbyActionFailedRequested.AddUniqueDynamic(
		this,
		&URiftLobbyWidget::HandleLobbyActionFailed
	);
}

void URiftLobbyWidget::UnbindFromPlayerController()
{
	ABasePlayerController* RiftPlayerController = BoundPlayerController.Get();
	if (!RiftPlayerController)
	{
		return;
	}

	RiftPlayerController->OnLobbyStartTransitionRequested.RemoveDynamic(
		this,
		&URiftLobbyWidget::HandleLobbyStartTransition
	);
	RiftPlayerController->OnLobbyActionFailedRequested.RemoveDynamic(
		this,
		&URiftLobbyWidget::HandleLobbyActionFailed
	);
	BoundPlayerController.Reset();
}

void URiftLobbyWidget::TryBindLobbyGameStateOrRetry()
{
	BindToLobbyGameState();

	if (BoundLobbyGameState.IsValid())
	{
		StopLobbyGameStateBindRetry();
		RefreshLobbyState();
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	if (!World->GetTimerManager().IsTimerActive(LobbyGameStateBindRetryTimerHandle))
	{
		World->GetTimerManager().SetTimer(
			LobbyGameStateBindRetryTimerHandle,
			this,
			&URiftLobbyWidget::TryBindLobbyGameStateOrRetry,
			0.1f,
			true
		);
	}
}

void URiftLobbyWidget::StopLobbyGameStateBindRetry()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	World->GetTimerManager().ClearTimer(LobbyGameStateBindRetryTimerHandle);
}

void URiftLobbyWidget::BindToLobbyGameState()
{
	UWorld* World = GetWorld();
	ARiftLobbyGameState* LobbyGameState = World ? World->GetGameState<ARiftLobbyGameState>() : nullptr;
	if (!LobbyGameState)
	{
		return;
	}

	if (BoundLobbyGameState.Get() == LobbyGameState)
	{
		return;
	}

	UnbindFromLobbyGameState();
	BoundLobbyGameState = LobbyGameState;
	LobbyGameState->OnLobbyGameStateChanged.AddUniqueDynamic(
		this,
		&URiftLobbyWidget::HandleLobbyGameStateChanged
	);
}

void URiftLobbyWidget::UnbindFromLobbyGameState()
{
	ARiftLobbyGameState* LobbyGameState = BoundLobbyGameState.Get();
	if (!LobbyGameState)
	{
		BoundLobbyGameState.Reset();
		return;
	}

	LobbyGameState->OnLobbyGameStateChanged.RemoveDynamic(
		this,
		&URiftLobbyWidget::HandleLobbyGameStateChanged
	);
	BoundLobbyGameState.Reset();
}

void URiftLobbyWidget::BindToLobbyPlayerStates()
{
	ABasePlayerController* RiftPlayerController = GetRiftPlayerController();
	ABasePlayerState* LocalPlayerState = RiftPlayerController
		? RiftPlayerController->GetPlayerState<ABasePlayerState>()
		: nullptr;
	if (LocalPlayerState)
	{
		LocalPlayerState->OnLobbyPlayerStateChanged.AddUniqueDynamic(
			this,
			&URiftLobbyWidget::HandleLobbyPlayerStateChanged
		);
		BoundPlayerStates.AddUnique(LocalPlayerState);
	}

	UWorld* World = GetWorld();
	const ARiftLobbyGameState* LobbyGameState = World ? World->GetGameState<ARiftLobbyGameState>() : nullptr;
	if (!LobbyGameState)
	{
		return;
	}

	for (APlayerState* PlayerState : LobbyGameState->PlayerArray)
	{
		ABasePlayerState* RiftPlayerState = Cast<ABasePlayerState>(PlayerState);
		if (!RiftPlayerState)
		{
			continue;
		}

		RiftPlayerState->OnLobbyPlayerStateChanged.AddUniqueDynamic(
			this,
			&URiftLobbyWidget::HandleLobbyPlayerStateChanged
		);
		BoundPlayerStates.AddUnique(RiftPlayerState);
	}
}

void URiftLobbyWidget::UnbindFromLobbyPlayerStates()
{
	for (const TWeakObjectPtr<ABasePlayerState>& PlayerStatePtr : BoundPlayerStates)
	{
		ABasePlayerState* RiftPlayerState = PlayerStatePtr.Get();
		if (!RiftPlayerState)
		{
			continue;
		}

		RiftPlayerState->OnLobbyPlayerStateChanged.RemoveDynamic(
			this,
			&URiftLobbyWidget::HandleLobbyPlayerStateChanged
		);
	}

	BoundPlayerStates.Empty();
}

void URiftLobbyWidget::RegisterLobbyDisplayActorsInWorld()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	for (TActorIterator<ARiftLobbyDisplayActor> DisplayActorIt(World); DisplayActorIt; ++DisplayActorIt)
	{
		RegisterLobbyDisplayActor(*DisplayActorIt);
	}
}

void URiftLobbyWidget::UpdateLobbyCameraForLocalState()
{
	if (IsLocalLobbyCharacterConfirmed())
	{
		StopLocalSlotCameraRetry();
		SwitchToGroupCamera();
		return;
	}

	SwitchToLocalSlotCamera();
}

void URiftLobbyWidget::HandleLobbyGameStateChanged()
{
	RefreshLobbyState();
}

void URiftLobbyWidget::HandleLobbyPlayerStateChanged()
{
	RefreshLobbyState();
}

void URiftLobbyWidget::HandleLobbyStartTransition(const float Duration)
{
	OnLobbyStartTransition(Duration);
}

void URiftLobbyWidget::HandleLobbyActionFailed(const FString& ErrorMessage)
{
	OnLobbyError(ErrorMessage);
}

void URiftLobbyWidget::RetryLocalSlotCameraSwitch()
{
	if (IsLocalLobbyCharacterConfirmed())
	{
		StopLocalSlotCameraRetry();
		return;
	}

	SwitchToLocalSlotCamera();
}

void URiftLobbyWidget::StopLocalSlotCameraRetry()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	World->GetTimerManager().ClearTimer(LocalSlotCameraRetryTimerHandle);
}

void URiftLobbyWidget::RefreshLobbyDisplayActor(ARiftLobbyDisplayActor* DisplayActor)
{
	if (!IsValid(DisplayActor))
	{
		return;
	}

	const int32 SlotIndex = DisplayActor->GetLobbySlotIndex();
	ABasePlayerState* PlayerState = GetPlayerStateByLobbySlotIndex(SlotIndex);
	if (!PlayerState)
	{
		DisplayActor->RefreshFromLobbyData(
			nullptr,
			ERiftLobbyDisplayMode::Empty,
			FRiftPlayerAppearanceSelection()
		);
		return;
	}

	const int32 LocalSlotIndex = GetLocalLobbySlotIndex();
	const bool bIsLocalSlot = SlotIndex == LocalSlotIndex;
	const bool bIsLocalCharacterConfirmed = IsLocalLobbyCharacterConfirmed();
	if (!bIsLocalSlot && !bIsLocalCharacterConfirmed)
	{
		DisplayActor->RefreshFromLobbyData(
			nullptr,
			ERiftLobbyDisplayMode::Empty,
			FRiftPlayerAppearanceSelection()
		);
		return;
	}

	if (PlayerState->IsLobbyCharacterConfirmed())
	{
		DisplayActor->RefreshFromLobbyData(
			PlayerState,
			ERiftLobbyDisplayMode::Confirmed,
			PlayerState->GetConfirmedAppearanceSelection()
		);
		return;
	}

	if (bIsLocalSlot)
	{
		DisplayActor->RefreshFromLobbyData(
			PlayerState,
			ERiftLobbyDisplayMode::LocalPreview,
			LocalDraftAppearanceSelection
		);
		return;
	}

	DisplayActor->RefreshFromLobbyData(
		PlayerState,
		ERiftLobbyDisplayMode::Ghost,
		FRiftPlayerAppearanceSelection()
	);
}
