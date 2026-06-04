// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "BaseHUD.generated.h"

class UPlayerHUDWidget;
/**
 *
 */
UCLASS()
class RIFTWARD_API ABaseHUD : public AHUD
{
	GENERATED_BODY()
public:
	ABaseHUD();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UI")
	TSubclassOf<UPlayerHUDWidget> PlayerInfoPanelClass;

	UPROPERTY(Transient)
	TObjectPtr<UPlayerHUDWidget> PlayerInfoPanel;

private:
	void CreatePlayerInfoPanel();
};
