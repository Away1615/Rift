// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PlayerAnimationConfig.generated.h"

class UBlendSpace;
class UAnimSequenceBase;

USTRUCT(BlueprintType)
struct FWeaponStanceAnimationSet
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UAnimSequenceBase> UnarmedIdle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UAnimSequenceBase> ArmedIdle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UAnimSequenceBase> Equip;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UAnimSequenceBase> Unequip;
};

UCLASS(BlueprintType)
class RIFTWARD_API UPlayerAnimationConfig : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion")
	TObjectPtr<UBlendSpace> GroundedLocomotionBlendSpace;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon")
	FWeaponStanceAnimationSet WeaponStance;
};
