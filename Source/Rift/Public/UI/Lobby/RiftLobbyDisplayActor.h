// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Appearance/RiftPlayerAppearanceTypes.h"
#include "GameFramework/Actor.h"
#include "RiftLobbyDisplayActor.generated.h"

class ABasePlayerState;
class UAnimationAsset;
class UPlayerAppearanceComponent;
class UPlayerClassConfig;
class USceneComponent;
class USkeletalMeshComponent;

UENUM(BlueprintType)
enum class ERiftLobbyDisplayMode : uint8
{
	Empty        UMETA(DisplayName="Empty"),
	Ghost        UMETA(DisplayName="Ghost"),
	LocalPreview UMETA(DisplayName="Local Preview"),
	Confirmed    UMETA(DisplayName="Confirmed")
};

UCLASS()
class RIFT_API ARiftLobbyDisplayActor : public AActor
{
	GENERATED_BODY()

public:
	ARiftLobbyDisplayActor();

	UFUNCTION(BlueprintPure, Category="Rift|Lobby")
	int32 GetLobbySlotIndex() const { return LobbySlotIndex; }

	UFUNCTION(BlueprintCallable, Category="Rift|Lobby")
	void SetLobbySlotIndex(int32 NewLobbySlotIndex);

	UFUNCTION(BlueprintPure, Category="Rift|Lobby")
	ABasePlayerState* GetCurrentPlayerState() const { return CurrentPlayerState.Get(); }

	UFUNCTION(BlueprintPure, Category="Rift|Lobby")
	ERiftLobbyDisplayMode GetCurrentDisplayMode() const { return CurrentDisplayMode; }

	UFUNCTION(BlueprintPure, Category="Rift|Lobby|Appearance")
	FRiftPlayerAppearanceSelection GetCurrentAppearanceSelection() const { return CurrentAppearanceSelection; }

	UFUNCTION(BlueprintCallable, Category="Rift|Lobby")
	void RefreshFromLobbyData(
		ABasePlayerState* PlayerState,
		ERiftLobbyDisplayMode DisplayMode,
		const FRiftPlayerAppearanceSelection& AppearanceSelection
	);

	UFUNCTION(BlueprintCallable, Category="Rift|Lobby|Preview")
	void SetPreviewRotationComponent(USceneComponent* NewPreviewRotationComponent);

	UFUNCTION(BlueprintCallable, Category="Rift|Lobby|Preview")
	void AddPreviewYaw(float DeltaYaw);

	UFUNCTION(BlueprintCallable, Category="Rift|Lobby|Preview")
	void SetPreviewYaw(float NewPreviewYaw);

	UFUNCTION(BlueprintCallable, Category="Rift|Lobby|Preview")
	void ResetPreviewYaw();

	UFUNCTION(BlueprintPure, Category="Rift|Lobby|Preview")
	float GetPreviewYaw() const { return PreviewYaw; }

	UFUNCTION(BlueprintPure, Category="Lobby|Camera")
	AActor* GetSlotCameraActor() const { return SlotCameraActor; }

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category="Rift|Lobby")
	int32 LobbySlotIndex = INDEX_NONE;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Lobby|Camera")
	TObjectPtr<AActor> SlotCameraActor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Rift|Lobby|Display", meta=(AllowPrivateAccess="true"))
	TObjectPtr<USceneComponent> SceneRootComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Rift|Lobby|Display", meta=(AllowPrivateAccess="true"))
	TObjectPtr<USceneComponent> PreviewPivotComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Rift|Lobby|Display", meta=(AllowPrivateAccess="true"))
	TObjectPtr<USkeletalMeshComponent> PreviewMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Rift|Lobby|Display|Appearance", meta=(AllowPrivateAccess="true"))
	TObjectPtr<USkeletalMeshComponent> HairMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Rift|Lobby|Display|Appearance", meta=(AllowPrivateAccess="true"))
	TObjectPtr<USkeletalMeshComponent> ArmUpperLeftMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Rift|Lobby|Display|Appearance", meta=(AllowPrivateAccess="true"))
	TObjectPtr<USkeletalMeshComponent> ArmUpperRightMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Rift|Lobby|Display", meta=(AllowPrivateAccess="true"))
	TObjectPtr<USkeletalMeshComponent> GhostMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Rift|Lobby|Display|Appearance", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UPlayerAppearanceComponent> AppearanceComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Rift|Lobby|Display", meta=(AllowPrivateAccess="true"))
	TArray<TObjectPtr<UAnimationAsset>> GhostAnimations;

protected:
	UFUNCTION(BlueprintImplementableEvent, Category="Rift|Lobby")
	void OnLobbyDisplayRefreshed(
		ABasePlayerState* PlayerState,
		ERiftLobbyDisplayMode DisplayMode,
		const FRiftPlayerAppearanceSelection& AppearanceSelection
	);

	UFUNCTION(BlueprintImplementableEvent, Category="Rift")
	void OnDisplayEmpty();

	UFUNCTION(BlueprintImplementableEvent, Category="Rift")
	void OnDisplayGhost(ABasePlayerState* PlayerState);

	UFUNCTION(BlueprintImplementableEvent, Category="Rift")
	void OnDisplayLocalPreview(
		ABasePlayerState* PlayerState,
		const FRiftPlayerAppearanceSelection& AppearanceSelection
	);

	UFUNCTION(BlueprintImplementableEvent, Category="Rift")
	void OnDisplayConfirmed(
		ABasePlayerState* PlayerState,
		const FRiftPlayerAppearanceSelection& AppearanceSelection
	);

private:
	static constexpr float DefaultPreviewYaw = -90.0f;

	USceneComponent* GetPreviewRotationComponent() const;
	void ApplyNativeDisplayMode(
		ABasePlayerState* PlayerState,
		ERiftLobbyDisplayMode DisplayMode,
		const FRiftPlayerAppearanceSelection& AppearanceSelection
	);
	void SetGhostVisible(bool bVisible);
	void SetPreviewVisible(bool bVisible);
	void PlayGhostAnimation();
	void StopGhostAnimation();
	void ApplyPreviewClass(UPlayerClassConfig* PreviewClassConfig);
	void ApplyPreviewAppearance(const FRiftPlayerAppearanceSelection& AppearanceSelection);
	void RefreshAppearanceLeaderPose();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Rift|Lobby|Preview", meta=(AllowPrivateAccess="true"))
	TObjectPtr<USceneComponent> PreviewRotationComponent;

	UPROPERTY(Transient)
	float PreviewYaw = DefaultPreviewYaw;

	UPROPERTY(Transient)
	TWeakObjectPtr<ABasePlayerState> CurrentPlayerState;

	UPROPERTY(Transient)
	ERiftLobbyDisplayMode CurrentDisplayMode = ERiftLobbyDisplayMode::Empty;

	UPROPERTY(Transient)
	FRiftPlayerAppearanceSelection CurrentAppearanceSelection;
};
