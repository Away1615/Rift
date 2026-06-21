#pragma once

#include "CoreMinimal.h"
#include "Data/Ability/RiftAbilityConfig.h"
#include "GuardAbilityConfig.generated.h"

class UAnimMontage;

UCLASS(BlueprintType, PrioritizeCategories=("Ability", "Input", "Guard"))
class RIFT_API UGuardAbilityConfig : public URiftAbilityConfig
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Guard")
	TObjectPtr<UAnimMontage> GuardMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Guard")
	FName GuardStartSection = TEXT("Start");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Guard")
	FName GuardLoopSection = TEXT("Loop");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Guard")
	FName GuardEndSection = TEXT("End");
};
