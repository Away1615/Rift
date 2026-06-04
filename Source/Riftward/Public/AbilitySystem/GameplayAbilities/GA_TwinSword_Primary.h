// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/GameplayAbilities/GA_TwinSword_Combo.h"
#include "GA_TwinSword_Primary.generated.h"

/**
 *
 */
UCLASS()
class RIFTWARD_API UGA_TwinSword_Primary : public UGA_TwinSword_Combo
{
	GENERATED_BODY()

public:
	UGA_TwinSword_Primary();

protected:
	virtual FGameplayTag GetComboWindowEventTag() const override;
	virtual FGameplayTag GetAbilityActiveStateTag() const override;
	virtual bool ShouldCommitComboAbility() const override;
};
