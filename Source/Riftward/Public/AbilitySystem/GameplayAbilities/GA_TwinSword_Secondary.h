// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/GameplayAbilities/GA_TwinSword_Combo.h"
#include "GA_TwinSword_Secondary.generated.h"

/**
 *
 */
UCLASS()
class RIFTWARD_API UGA_TwinSword_Secondary : public UGA_TwinSword_Combo
{
	GENERATED_BODY()

public:
	UGA_TwinSword_Secondary();

protected:
	virtual FGameplayTag GetComboWindowEventTag() const override;
	virtual FGameplayTag GetAbilityActiveStateTag() const override;
	virtual bool ShouldCommitComboAbility() const override;
};
