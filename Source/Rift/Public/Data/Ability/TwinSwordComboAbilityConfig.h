#pragma once

#include "CoreMinimal.h"
#include "Data/Ability/RiftAbilityConfig.h"
#include "TwinSwordComboAbilityConfig.generated.h"

class URiftComboGraph;

UCLASS(BlueprintType, PrioritizeCategories=("Ability", "Input", "Combo", "Facing", "RapidSlash"))
class RIFT_API UTwinSwordComboAbilityConfig : public URiftAbilityConfig
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combo")
	TObjectPtr<URiftComboGraph> ComboGraph;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combo")
	float ComboInputBufferDuration = 0.25f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Facing")
	float AssistFacingDuration = 0.12f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Facing")
	float AssistFacingRotationSpeed = 1440.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="RapidSlash")
	FName RapidSlashReadyGrantSection = TEXT("Light_4");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="RapidSlash")
	float RapidSlashReadyDuration = 2.0f;
};
