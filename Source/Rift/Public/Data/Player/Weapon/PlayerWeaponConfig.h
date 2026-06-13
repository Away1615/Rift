// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Combat/RiftWeaponTraceComponent.h"
#include "Engine/DataAsset.h"
#include "PlayerWeaponConfig.generated.h"

class UStaticMesh;

USTRUCT(BlueprintType)
struct FPlayerWeaponPartConfig
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon")
	TObjectPtr<UStaticMesh> WeaponMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon")
	ERiftWeaponSlot WeaponSlot = ERiftWeaponSlot::Left;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon")
	float TraceRadius = 15.0f;
};

UCLASS(BlueprintType)
class RIFT_API UPlayerWeaponConfig : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon")
	TArray<FPlayerWeaponPartConfig> EquippedWeapons;
};
