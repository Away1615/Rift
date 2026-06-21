#pragma once

#include "CoreMinimal.h"
#include "Combat/RiftPlayerHitReactionTypes.h"
#include "Data/Ability/RiftAbilityConfig.h"
#include "EnemyMeleeAttackAbilityConfig.generated.h"

class UAnimMontage;
class URiftCombatCueConfig;

UCLASS(BlueprintType, PrioritizeCategories=("Ability", "Melee", "Damage", "Feedback", "Poise"))
class RIFT_API UEnemyMeleeAttackAbilityConfig : public URiftAbilityConfig
{
	GENERATED_BODY()

public:
	UEnemyMeleeAttackAbilityConfig();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Melee")
	TObjectPtr<UAnimMontage> AttackMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Melee")
	float AttackRange = 200.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Melee")
	float AttackCooldown = 2.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Melee")
	float HitboxRadius = 120.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Melee")
	float HitboxForwardOffset = 100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Damage")
	float AttackDamage = 15.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Damage")
	ERiftPlayerHitReaction PlayerHitReaction = ERiftPlayerHitReaction::IndicatorOnly;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Feedback")
	TObjectPtr<URiftCombatCueConfig> CombatCueConfig;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Poise")
	float PoiseBreakStaggerDuration = 1.2f;
};
