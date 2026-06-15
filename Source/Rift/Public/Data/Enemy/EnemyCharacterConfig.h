// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "Combat/RiftWeaponTypes.h"
#include "Engine/DataAsset.h"
#include "EnemyCharacterConfig.generated.h"

class UEnemyAnimationConfig;
class UEnemyCombatConfig;
class UStaticMesh;

UCLASS(BlueprintType)
class RIFT_API UEnemyCharacterConfig : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Attribute|Health")
	float Health = 100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Attribute|Health")
	float MaxHealth = 100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Attribute|Poise")
	float Poise = 100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Attribute|Poise")
	float MaxPoise = 100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Attribute|Poise")
	float PoiseRegenDelay = 2.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion")
	float MoveSpeed = 350.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon")
	TMap<ERiftWeaponSlot, TObjectPtr<UStaticMesh>> Weapons;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Death")
	float KillUltimateCharge = 3.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Death")
	float DeathDespawnDelay = 3.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Animation")
	TObjectPtr<UEnemyAnimationConfig> EnemyAnimationConfig;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat")
	TObjectPtr<UEnemyCombatConfig> EnemyCombatConfig;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability")
	TArray<TSubclassOf<UGameplayAbility>> GrantedAbilities;
};
