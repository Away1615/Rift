#pragma once

#include "CoreMinimal.h"
#include "Combat/RiftDamageReactionTypes.h"
#include "Engine/DataAsset.h"
#include "EnemyCombatConfig.generated.h"

class UAnimMontage;

UCLASS(BlueprintType)
class RIFT_API UEnemyCombatConfig : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Attack")
	TObjectPtr<UAnimMontage> AttackMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Attack")
	float AttackDamage = 15.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Attack")
	ERiftPlayerDamageReactionType PlayerDamageReaction = ERiftPlayerDamageReactionType::Hit;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Attack")
	float AttackRange = 200.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Attack")
	float AttackCooldown = 2.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Attack")
	float HitboxRadius = 120.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Attack")
	float HitboxForwardOffset = 100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Poise")
	float PoiseBreakStaggerDuration = 1.2f;
};
