// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Lobby/RiftAppearanceOptionItemObject.h"

#include "UI/Lobby/RiftLobbyWidget.h"

void URiftAppearanceOptionItemObject::InitializeAppearanceOption(
	URiftLobbyWidget* InOwningLobbyWidget,
	const ERiftPlayerAppearanceSlot InSlot,
	const FName InPartId,
	const FText& InDisplayName
)
{
	OwningLobbyWidget = InOwningLobbyWidget;
	Slot = InSlot;
	PartId = InPartId;
	DisplayName = InDisplayName;
}

void URiftAppearanceOptionItemObject::SelectAppearanceOption()
{
	if (!OwningLobbyWidget)
	{
		return;
	}

	OwningLobbyWidget->SetLocalDraftAppearancePart(Slot, PartId);
}
