// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/BaseGameMode.h"
#include "Core/BaseGameState.h"
#include "Character/PlayerCharacter.h"
#include "Player/BaseHUD.h"
#include "Player/BasePlayerController.h"
#include "Player/BasePlayerState.h"

ABaseGameMode::ABaseGameMode()
{

    DefaultPawnClass = APlayerCharacter::StaticClass();
    HUDClass = ABaseHUD::StaticClass();
    PlayerControllerClass = ABasePlayerController::StaticClass();
    GameStateClass = ABaseGameState::StaticClass();
    PlayerStateClass = ABasePlayerState::StaticClass();
    
}

