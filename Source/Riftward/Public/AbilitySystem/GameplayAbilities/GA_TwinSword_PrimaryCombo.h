// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/GameplayAbilities/GA_TwinSword_MeleeComboBase.h"
#include "GA_TwinSword_PrimaryCombo.generated.h"

UCLASS()
class RIFTWARD_API UGA_TwinSword_PrimaryCombo : public UGA_TwinSword_MeleeComboBase
{
	GENERATED_BODY()

public:
	UGA_TwinSword_PrimaryCombo();

protected:
	virtual FGameplayTag GetAbilityActiveStateTag() const override;
	virtual bool ShouldCommitComboAbility() const override;
};
