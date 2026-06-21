#pragma once

#include "CoreMinimal.h"
#include "Data/Ability/RiftAbilityConfig.h"
#include "DodgeAbilityConfig.generated.h"

class UAnimMontage;

UCLASS(BlueprintType, PrioritizeCategories=("Ability", "Input", "Dodge"))
class RIFT_API UDodgeAbilityConfig : public URiftAbilityConfig
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Dodge")
	TObjectPtr<UAnimMontage> DodgeMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Dodge|Perfect")
	float PerfectDodgeWindowDuration = 0.2f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Dodge|Perfect")
	float PerfectDodgeUltimateChargeReward = 8.0f;
};
