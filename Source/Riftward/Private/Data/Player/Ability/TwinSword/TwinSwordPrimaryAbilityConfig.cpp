// Fill out your copyright notice in the Description page of Project Settings.



#include "Data/Player/Ability/TwinSword/TwinSwordPrimaryAbilityConfig.h"
#include "AbilitySystem/GameplayAbilities/GA_TwinSword_PrimaryCombo.h"
#include "AbilitySystem/GameplayEffects/GE_InstantDamage.h"

UTwinSwordPrimaryAbilityConfig::UTwinSwordPrimaryAbilityConfig()
{
	AbilityClass = UGA_TwinSword_PrimaryCombo::StaticClass();
	InstantDamageEffectClass = UGE_InstantDamage::StaticClass();
	ComboSections = {
		FName(TEXT("Attack1")),
		FName(TEXT("Attack2")),
		FName(TEXT("Attack3"))
	};
}
