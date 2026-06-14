#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "RiftComboGraph.generated.h"

class UAnimMontage;
class UCameraShakeBase;

USTRUCT(BlueprintType)
struct FRiftComboTransition
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(Categories="InputTag"))
	FGameplayTag InputTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName ToSection;
};

USTRUCT(BlueprintType)
struct FRiftComboNode
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName SectionName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float Damage = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float PoiseDamage = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float StaminaCost = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float SwordIntentOnHit = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float UltimateChargeOnHit = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UCameraShakeBase> CameraShake;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FVector2D CameraShakeDir = FVector2D(1.0f, 0.0f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FRiftComboTransition> Transitions;
};

USTRUCT(BlueprintType)
struct FRiftComboEntry
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(Categories="InputTag"))
	FGameplayTag InputTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName EntrySection;
};

UCLASS(BlueprintType)
class RIFT_API URiftComboGraph : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combo")
	TObjectPtr<UAnimMontage> Montage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combo")
	TArray<FRiftComboEntry> Entries;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combo")
	TArray<FRiftComboNode> Nodes;

	const FRiftComboNode* FindNode(FName SectionName) const;
	FName GetEntrySection(const FGameplayTag& InputTag) const;
};
