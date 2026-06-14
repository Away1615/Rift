// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/BaseHUD.h"
#include "UI/Player/PlayerHUDWidget.h"

ABaseHUD::ABaseHUD()
{
}

void ABaseHUD::BeginPlay()
{
	Super::BeginPlay();
	CreatePlayerInfoPanel();
}

void ABaseHUD::CreatePlayerInfoPanel()
{
	APlayerController* OwningPC = GetOwningPlayerController();

	if (!OwningPC) return;

	if (!PlayerInfoPanelClass)
	{


	}
}
