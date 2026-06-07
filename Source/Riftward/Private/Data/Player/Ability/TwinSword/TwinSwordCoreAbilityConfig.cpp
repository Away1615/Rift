// Fill out your copyright notice in the Description page of Project Settings.


#include "Data/Player/Ability/TwinSword/TwinSwordCoreAbilityConfig.h"

#include "AbilitySystem/GameplayAbilities/GA_TwinSword_Dodge.h"

UTwinSwordCoreAbilityConfig::UTwinSwordCoreAbilityConfig()
{
	AbilityClass = UGA_TwinSword_Dodge::StaticClass();
}
