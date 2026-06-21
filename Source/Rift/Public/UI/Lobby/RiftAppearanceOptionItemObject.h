// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Appearance/RiftPlayerAppearanceTypes.h"
#include "UObject/Object.h"
#include "RiftAppearanceOptionItemObject.generated.h"

class URiftLobbyWidget;

UCLASS(BlueprintType)
class RIFT_API URiftAppearanceOptionItemObject : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="Rift|Lobby|Appearance")
	void InitializeAppearanceOption(
		URiftLobbyWidget* InOwningLobbyWidget,
		ERiftPlayerAppearanceSlot InSlot,
		FName InPartId,
		const FText& InDisplayName
	);

	UFUNCTION(BlueprintCallable, Category="Rift|Lobby|Appearance")
	void SelectAppearanceOption();

	UFUNCTION(BlueprintPure, Category="Rift|Lobby|Appearance")
	ERiftPlayerAppearanceSlot GetSlot() const { return Slot; }

	UFUNCTION(BlueprintPure, Category="Rift|Lobby|Appearance")
	FName GetPartId() const { return PartId; }

	UFUNCTION(BlueprintPure, Category="Rift|Lobby|Appearance")
	FText GetDisplayName() const { return DisplayName; }

protected:
	UPROPERTY(BlueprintReadOnly, Category="Rift|Lobby|Appearance")
	TObjectPtr<URiftLobbyWidget> OwningLobbyWidget;

	UPROPERTY(BlueprintReadOnly, Category="Rift|Lobby|Appearance")
	ERiftPlayerAppearanceSlot Slot = ERiftPlayerAppearanceSlot::Hair;

	UPROPERTY(BlueprintReadOnly, Category="Rift|Lobby|Appearance")
	FName PartId = NAME_None;

	UPROPERTY(BlueprintReadOnly, Category="Rift|Lobby|Appearance")
	FText DisplayName;
};
