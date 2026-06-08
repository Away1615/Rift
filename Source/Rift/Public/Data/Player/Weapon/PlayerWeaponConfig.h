// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PlayerWeaponConfig.generated.h"

class APlayerWeapon;

USTRUCT(BlueprintType)
struct FPlayerWeaponPartConfig
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon")
	TSubclassOf<APlayerWeapon> WeaponActorClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon")
	FName AttachSocket = NAME_None;
};

UCLASS(BlueprintType)
class RIFT_API UPlayerWeaponConfig : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon")
	TArray<FPlayerWeaponPartConfig> EquippedWeapons;
};
