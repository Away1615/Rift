#pragma once

#include "CoreMinimal.h"
#include "Appearance/RiftPlayerAppearanceTypes.h"
#include "GameFramework/Actor.h"
#include "RiftLobbyPreviewCharacter.generated.h"

class UPlayerAppearanceComponent;
class UPlayerClassConfig;
class UDataTable;
class USceneComponent;
class USkeletalMeshComponent;

UCLASS(Blueprintable)
class RIFT_API ARiftLobbyPreviewCharacter : public AActor
{
	GENERATED_BODY()

public:
	ARiftLobbyPreviewCharacter();

	UFUNCTION(BlueprintCallable, Category="Lobby|Preview")
	void ApplyPreviewClass(UPlayerClassConfig* PreviewClassConfig);

	UFUNCTION(BlueprintCallable, Category="Lobby|Preview")
	bool ApplyPreviewAppearance(const FRiftPlayerAppearanceSelection& AppearanceSelection);

	UFUNCTION(BlueprintCallable, Category="Lobby|Preview")
	void ClearPreviewAppearance();

	UFUNCTION(BlueprintCallable, Category="Lobby|Preview")
	void RefreshAppearanceLeaderPose();

	UFUNCTION(BlueprintPure, Category="Lobby|Preview")
	USceneComponent* GetPreviewPivotComponent() const { return PreviewPivotComponent; }

	UFUNCTION(BlueprintPure, Category="Lobby|Preview")
	USkeletalMeshComponent* GetPreviewMeshComponent() const { return PreviewMeshComponent; }

	UFUNCTION(BlueprintPure, Category="Lobby|Preview")
	UPlayerAppearanceComponent* GetAppearanceComponent() const { return AppearanceComponent; }

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Lobby|Preview|Appearance")
	TObjectPtr<UDataTable> AppearanceTable;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Lobby|Preview")
	TObjectPtr<USceneComponent> SceneRootComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Lobby|Preview")
	TObjectPtr<USceneComponent> PreviewPivotComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Lobby|Preview")
	TObjectPtr<USkeletalMeshComponent> PreviewMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Lobby|Preview|Appearance")
	TObjectPtr<USkeletalMeshComponent> HairMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Lobby|Preview|Appearance")
	TObjectPtr<USkeletalMeshComponent> ArmUpperLeftMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Lobby|Preview|Appearance")
	TObjectPtr<USkeletalMeshComponent> ArmUpperRightMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Lobby|Preview|Appearance")
	TObjectPtr<UPlayerAppearanceComponent> AppearanceComponent;

private:
	void SyncAppearanceTable();
};
