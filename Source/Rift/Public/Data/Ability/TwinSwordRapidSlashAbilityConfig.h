#pragma once

#include "CoreMinimal.h"
#include "Combat/RiftCombatFeedbackTypes.h"
#include "Data/Ability/RiftAbilityConfig.h"
#include "TwinSwordRapidSlashAbilityConfig.generated.h"

class UAnimMontage;
class UCameraShakeBase;
class UNiagaraSystem;
class URiftCombatCueConfig;

UCLASS(BlueprintType, PrioritizeCategories=("Ability", "Input", "RapidSlash", "Damage", "Feedback", "Facing", "Aura"))
class RIFT_API UTwinSwordRapidSlashAbilityConfig : public URiftAbilityConfig
{
	GENERATED_BODY()

public:
	UTwinSwordRapidSlashAbilityConfig();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="RapidSlash")
	TObjectPtr<UAnimMontage> Montage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="RapidSlash")
	FName SectionA = TEXT("Rapid_1");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="RapidSlash")
	FName SectionB = TEXT("Rapid_2");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="RapidSlash")
	FName FinisherSection = TEXT("Rapid_Finisher");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="RapidSlash")
	float PlayRate = 1.35f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Facing")
	float AssistFacingDuration = 0.12f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Facing")
	float AssistFacingRotationSpeed = 1440.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Damage")
	float Damage = 8.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Damage")
	float PoiseDamage = 5.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Damage")
	float UltimateChargeOnHit = 3.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Damage")
	float FinisherDamage = 28.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Damage")
	float FinisherPoiseDamage = 45.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Damage")
	float FinisherUltimateChargeOnHit = 6.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Feedback")
	FRiftMeleeHitStopConfig HitStopConfig;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Feedback")
	TObjectPtr<URiftCombatCueConfig> CombatCueConfig;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Feedback")
	TSubclassOf<UCameraShakeBase> CameraShake;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Feedback")
	FVector2D CameraShakeDir = FVector2D(1.0f, 0.0f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Feedback")
	FRiftMeleeHitStopConfig FinisherHitStopConfig;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Feedback")
	TObjectPtr<URiftCombatCueConfig> FinisherCombatCueConfig;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Feedback")
	TSubclassOf<UCameraShakeBase> FinisherCameraShake;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Feedback")
	FVector2D FinisherCameraShakeDir = FVector2D(1.0f, 0.0f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Aura")
	TObjectPtr<UNiagaraSystem> RapidSlashAuraNiagara;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Aura")
	FName RapidSlashAuraAttachSocketName = NAME_None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Aura")
	FVector RapidSlashAuraLocationOffset = FVector::ZeroVector;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Aura")
	FRotator RapidSlashAuraRotationOffset = FRotator::ZeroRotator;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Aura")
	FVector RapidSlashAuraScale = FVector(1.0f);
};
