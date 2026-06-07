#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/GameplayAbilities/GA_TwinSword_MeleeComboBase.h"
#include "GA_TwinSword_PrimaryHeavy.generated.h"

UCLASS()
class RIFTWARD_API UGA_TwinSword_PrimaryHeavy : public UGA_TwinSword_MeleeComboBase
{
	GENERATED_BODY()

public:
	UGA_TwinSword_PrimaryHeavy();

protected:
	virtual FGameplayTag GetAbilityActiveStateTag() const override;
	virtual bool ShouldCommitComboAbility() const override;
};
