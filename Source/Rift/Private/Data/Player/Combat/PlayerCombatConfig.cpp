// Fill out your copyright notice in the Description page of Project Settings.

#include "Data/Player/Combat/PlayerCombatConfig.h"

UPlayerCombatConfig::UPlayerCombatConfig()
{
	PrimaryAttackSections = {
		TEXT("Attack_1"),
		TEXT("Attack_2"),
		TEXT("Attack_3"),
		TEXT("Attack_4")
	};
}
