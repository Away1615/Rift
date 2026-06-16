// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Menu/RiftMainMenuWidget.h"

#include "Kismet/KismetSystemLibrary.h"

void URiftMainMenuWidget::QuitGame()
{
	UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, true);
}
