// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "AbilityEffectConfig.generated.h"

UENUM(BlueprintType)
enum class EAbilityEffectKind : uint8
{
	Damage					UMETA(DisplayName="Damage"),
	Heal					UMETA(DisplayName="Heal"),
	CooldownReduction		UMETA(DisplayName="Cooldown Reduction"),
	DamageMultiplier		UMETA(DisplayName="Damage Multiplier"),
	AttackSpeedMultiplier	UMETA(DisplayName="Attack Speed Multiplier"),
	AttackRangeMultiplier	UMETA(DisplayName="Attack Range Multiplier"),
	SwordWave				UMETA(DisplayName="Sword Wave"),
	PhantomDamage			UMETA(DisplayName="Phantom Damage")
};

UENUM(BlueprintType)
enum class EAbilityEffectTriggerEvent : uint8
{
	OnApply	UMETA(DisplayName="On Apply"),
	OnHit	UMETA(DisplayName="On Hit")
};

USTRUCT(BlueprintType)
struct FAbilityEffectConfig
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Effect")
	EAbilityEffectKind Type = EAbilityEffectKind::Damage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Effect")
	EAbilityEffectTriggerEvent Trigger = EAbilityEffectTriggerEvent::OnHit;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Effect", meta=(ClampMin="0.0"))
	float Magnitude = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Effect", meta=(ClampMin="0.0"))
	float Duration = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Effect|Condition")
	FGameplayTag RequiredOwnerTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Effect|Cooldown")
	FGameplayTagContainer CooldownTags;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Effect|Area", meta=(ClampMin="0.0"))
	float Range = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Effect|Area", meta=(ClampMin="0.0"))
	float Radius = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Effect|Debug")
	bool bDrawDebug = false;
};
