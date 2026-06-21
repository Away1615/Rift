// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RiftRoomCodePopupWidget.generated.h"

class URiftGameInstance;

UENUM(BlueprintType)
enum class ERiftRoomCodePopupMode : uint8
{
	CreateRoom UMETA(DisplayName="Create Room"),
	JoinRoom   UMETA(DisplayName="Join Room")
};

UCLASS()
class RIFT_API URiftRoomCodePopupWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="Rift|Room")
	void SetPopupMode(ERiftRoomCodePopupMode NewPopupMode);

	UFUNCTION(BlueprintPure, Category="Rift|Room")
	ERiftRoomCodePopupMode GetPopupMode() const { return PopupMode; }

	UFUNCTION(BlueprintPure, Category="Rift|Room")
	bool IsLoading() const { return bIsLoading; }

	UFUNCTION(BlueprintPure, Category="Rift|Room")
	FString GetLastErrorMessage() const { return LastErrorMessage; }

	UFUNCTION(BlueprintCallable, Category="Rift|Room")
	void SubmitRoomCode(const FString& RoomCode);

	UFUNCTION(BlueprintCallable, Category="Rift|Room")
	void CancelPopup();

	UFUNCTION(BlueprintCallable, Category="Rift|Room")
	void ClosePopup();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Rift|Room")
	ERiftRoomCodePopupMode PopupMode = ERiftRoomCodePopupMode::JoinRoom;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Rift|Room")
	bool bCloseOnSuccess = true;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION(BlueprintImplementableEvent, Category="Rift|Room")
	void OnLoadingChanged(bool bLoading);

	UFUNCTION(BlueprintImplementableEvent, Category="Rift|Room")
	void OnRoomError(const FString& ErrorMessage);

	UFUNCTION(BlueprintImplementableEvent, Category="Rift|Room")
	void OnRoomSucceeded(const FString& RoomCode);

	UFUNCTION(BlueprintImplementableEvent, Category="Rift|Room")
	void OnPopupClosed();

private:
	URiftGameInstance* GetRiftGameInstance() const;
	void BindToGameInstance();
	void UnbindFromGameInstance();
	void SetLoading(bool bLoading);
	void HandleRelevantFailure(const FString& ErrorMessage);
	void HandleRelevantSuccess(const FString& RoomCode);

	UFUNCTION()
	void HandleCreateRoomSucceeded(const FString& RoomCode);

	UFUNCTION()
	void HandleCreateRoomFailed(const FString& ErrorMessage);

	UFUNCTION()
	void HandleJoinRoomSucceeded(const FString& RoomCode);

	UFUNCTION()
	void HandleJoinRoomFailed(const FString& ErrorMessage);

	UFUNCTION()
	void HandleFindRoomFailed(const FString& ErrorMessage);

	UPROPERTY(Transient)
	TObjectPtr<URiftGameInstance> BoundGameInstance;

	bool bIsLoading = false;
	FString LastErrorMessage;
};
