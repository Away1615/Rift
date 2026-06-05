// Fill out your copyright notice in the Description page of Project Settings.


#include "Data/Player/Ability/TwinSword/TwinSwordSecondaryAbilityConfig.h"

#include "AbilitySystem/GameplayAbilities/GA_TwinSword_Secondary.h"
#include "AbilitySystem/GameplayEffects/GE_InstantDamage.h"
#include "AbilitySystem/GameplayEffects/GE_StaminaCost.h"

UTwinSwordSecondaryAbilityConfig::UTwinSwordSecondaryAbilityConfig()
{
	AbilityClass = UGA_TwinSword_Secondary::StaticClass();
	InstantDamageEffectClass = UGE_InstantDamage::StaticClass();
	StaminaCostEffectClass = UGE_StaminaCost::StaticClass();
	StaminaCost = 20.0f;
	ComboSections = {
		FName(TEXT("Attack1")),
		FName(TEXT("Attack2")),
		FName(TEXT("Attack3")),
		FName(TEXT("Attack4"))
	};
}
