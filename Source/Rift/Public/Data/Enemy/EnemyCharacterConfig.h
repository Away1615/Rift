// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Combat/RiftWeaponTypes.h"
#include "Engine/DataAsset.h"
#include "EnemyCharacterConfig.generated.h"

class UAnimInstance;
class UAnimMontage;
class URiftAbilityConfig;
class UBehaviorTree;
class USkeletalMesh;
class UStaticMesh;

UCLASS(BlueprintType)
class RIFT_API UEnemyCharacterConfig : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Attribute")
	float Health = 100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Attribute")
	float MaxHealth = 100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Attribute")
	float Poise = 100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Attribute")
	float MaxPoise = 100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Attribute")
	float PoiseRegenDelay = 2.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Attribute")
	float MoveSpeed = 350.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Appearance")
	TObjectPtr<USkeletalMesh> SkeletalMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Appearance")
	TObjectPtr<UStaticMesh> HeadMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Animation")
	TSubclassOf<UAnimInstance> AnimClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Animation")
	TObjectPtr<UAnimMontage> SpawnIntroMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Animation")
	TObjectPtr<UAnimMontage> HitMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Animation")
	TObjectPtr<UAnimMontage> StaggeredMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Animation")
	TObjectPtr<UAnimMontage> DeathMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="HitReaction|Retreat")
	TObjectPtr<UAnimMontage> HitRetreatMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="HitReaction|Retreat")
	float HitRetreatCooldown = 1.2f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="HitReaction|Retreat", meta=(ClampMin="0.0", ClampMax="1.0"))
	float HitRetreatChance = 0.2f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="HitReaction|Retreat", meta=(ClampMin="0.0", ClampMax="1.0"))
	float BlockingHitRetreatChance = 0.8f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="AI")
	TObjectPtr<UBehaviorTree> BehaviorTree;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="AI")
	float TargetSearchRadius = 1500.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon")
	TMap<ERiftWeaponSlot, TObjectPtr<UStaticMesh>> Weapons;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Death")
	float KillUltimateCharge = 3.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Death")
	float DeathDespawnDelay = 3.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability")
	TArray<TObjectPtr<URiftAbilityConfig>> AbilityConfigs;
};
