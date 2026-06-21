// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Menu/RiftRoomCodePopupWidget.h"

#include "Core/RiftGameInstance.h"

void URiftRoomCodePopupWidget::NativeConstruct()
{
	Super::NativeConstruct();

	BindToGameInstance();
	OnLoadingChanged(bIsLoading);
}

void URiftRoomCodePopupWidget::NativeDestruct()
{
	UnbindFromGameInstance();

	Super::NativeDestruct();
}

void URiftRoomCodePopupWidget::SetPopupMode(const ERiftRoomCodePopupMode NewPopupMode)
{
	PopupMode = NewPopupMode;
}

void URiftRoomCodePopupWidget::SubmitRoomCode(const FString& RoomCode)
{
	if (bIsLoading)
	{
		return;
	}

	URiftGameInstance* RiftGameInstance = GetRiftGameInstance();
	if (!RiftGameInstance)
	{
		HandleRelevantFailure(TEXT("Game instance is unavailable."));
		return;
	}

	LastErrorMessage.Empty();
	SetLoading(true);

	if (PopupMode == ERiftRoomCodePopupMode::CreateRoom)
	{
		RiftGameInstance->CreateLanRoom(RoomCode);
	}
	else
	{
		RiftGameInstance->JoinLanRoomByCode(RoomCode);
	}
}

void URiftRoomCodePopupWidget::CancelPopup()
{
	SetLoading(false);
	ClosePopup();
}

void URiftRoomCodePopupWidget::ClosePopup()
{
	OnPopupClosed();
	RemoveFromParent();
}

URiftGameInstance* URiftRoomCodePopupWidget::GetRiftGameInstance() const
{
	return GetGameInstance<URiftGameInstance>();
}

void URiftRoomCodePopupWidget::BindToGameInstance()
{
	URiftGameInstance* RiftGameInstance = GetRiftGameInstance();
	if (!RiftGameInstance || BoundGameInstance == RiftGameInstance)
	{
		return;
	}

	UnbindFromGameInstance();
	BoundGameInstance = RiftGameInstance;

	RiftGameInstance->OnCreateRoomSucceeded.AddUniqueDynamic(
		this,
		&URiftRoomCodePopupWidget::HandleCreateRoomSucceeded
	);
	RiftGameInstance->OnCreateRoomFailed.AddUniqueDynamic(
		this,
		&URiftRoomCodePopupWidget::HandleCreateRoomFailed
	);
	RiftGameInstance->OnJoinRoomSucceeded.AddUniqueDynamic(
		this,
		&URiftRoomCodePopupWidget::HandleJoinRoomSucceeded
	);
	RiftGameInstance->OnJoinRoomFailed.AddUniqueDynamic(
		this,
		&URiftRoomCodePopupWidget::HandleJoinRoomFailed
	);
	RiftGameInstance->OnFindRoomFailed.AddUniqueDynamic(
		this,
		&URiftRoomCodePopupWidget::HandleFindRoomFailed
	);
}

void URiftRoomCodePopupWidget::UnbindFromGameInstance()
{
	URiftGameInstance* RiftGameInstance = BoundGameInstance.Get();
	if (!RiftGameInstance)
	{
		BoundGameInstance = nullptr;
		return;
	}

	RiftGameInstance->OnCreateRoomSucceeded.RemoveDynamic(
		this,
		&URiftRoomCodePopupWidget::HandleCreateRoomSucceeded
	);
	RiftGameInstance->OnCreateRoomFailed.RemoveDynamic(
		this,
		&URiftRoomCodePopupWidget::HandleCreateRoomFailed
	);
	RiftGameInstance->OnJoinRoomSucceeded.RemoveDynamic(
		this,
		&URiftRoomCodePopupWidget::HandleJoinRoomSucceeded
	);
	RiftGameInstance->OnJoinRoomFailed.RemoveDynamic(
		this,
		&URiftRoomCodePopupWidget::HandleJoinRoomFailed
	);
	RiftGameInstance->OnFindRoomFailed.RemoveDynamic(
		this,
		&URiftRoomCodePopupWidget::HandleFindRoomFailed
	);

	BoundGameInstance = nullptr;
}

void URiftRoomCodePopupWidget::SetLoading(const bool bLoading)
{
	if (bIsLoading == bLoading)
	{
		return;
	}

	bIsLoading = bLoading;
	OnLoadingChanged(bIsLoading);
}

void URiftRoomCodePopupWidget::HandleRelevantFailure(const FString& ErrorMessage)
{
	LastErrorMessage = ErrorMessage;
	SetLoading(false);
	OnRoomError(ErrorMessage);
}

void URiftRoomCodePopupWidget::HandleRelevantSuccess(const FString& RoomCode)
{
	SetLoading(false);
	OnRoomSucceeded(RoomCode);

	if (bCloseOnSuccess)
	{
		ClosePopup();
	}
}

void URiftRoomCodePopupWidget::HandleCreateRoomSucceeded(const FString& RoomCode)
{
	if (PopupMode != ERiftRoomCodePopupMode::CreateRoom)
	{
		return;
	}

	HandleRelevantSuccess(RoomCode);
}

void URiftRoomCodePopupWidget::HandleCreateRoomFailed(const FString& ErrorMessage)
{
	if (PopupMode != ERiftRoomCodePopupMode::CreateRoom)
	{
		return;
	}

	HandleRelevantFailure(ErrorMessage);
}

void URiftRoomCodePopupWidget::HandleJoinRoomSucceeded(const FString& RoomCode)
{
	if (PopupMode != ERiftRoomCodePopupMode::JoinRoom)
	{
		return;
	}

	HandleRelevantSuccess(RoomCode);
}

void URiftRoomCodePopupWidget::HandleJoinRoomFailed(const FString& ErrorMessage)
{
	if (PopupMode != ERiftRoomCodePopupMode::JoinRoom)
	{
		return;
	}

	HandleRelevantFailure(ErrorMessage);
}

void URiftRoomCodePopupWidget::HandleFindRoomFailed(const FString& ErrorMessage)
{
	if (PopupMode != ERiftRoomCodePopupMode::JoinRoom)
	{
		return;
	}

	HandleRelevantFailure(ErrorMessage);
}
