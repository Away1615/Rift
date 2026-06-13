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

	PrimaryAttackSectionDamage = {
		10.0f,
		10.0f,
		12.0f,
		18.0f
	};

	PrimaryAttackSectionPoiseDamage = {
		10.0f,
		10.0f,
		12.0f,
		18.0f
	};

	PrimaryAttackSectionShakeDir = {
		FVector2D(1.0f, 0.0f),
		FVector2D(-1.0f, 0.0f),
		FVector2D(1.0f, 0.0f),
		FVector2D(0.0f, 1.0f)
	};
}
