// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RiftMainMenuWidget.generated.h"

UCLASS()
class RIFT_API URiftMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="Rift|Menu")
	void QuitGame();
};
