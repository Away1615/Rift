#pragma once

#include "CoreMinimal.h"
#include "Data/Ability/RiftAbilityConfig.h"
#include "EnemyShieldBlockAbilityConfig.generated.h"

class UAnimMontage;

UCLASS(BlueprintType, PrioritizeCategories=("Ability", "ShieldBlock"))
class RIFT_API UEnemyShieldBlockAbilityConfig : public URiftAbilityConfig
{
	GENERATED_BODY()

public:
	UEnemyShieldBlockAbilityConfig();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="ShieldBlock")
	TObjectPtr<UAnimMontage> ShieldBlockMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="ShieldBlock")
	float ShieldBlockCooldown = 3.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="ShieldBlock", meta=(ClampMin="0.0", ClampMax="1.0"))
	float BlockingPoiseDamageMultiplier = 0.5f;
};
