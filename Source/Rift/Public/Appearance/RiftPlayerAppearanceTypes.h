// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Engine/SkeletalMesh.h"
#include "RiftPlayerAppearanceTypes.generated.h"

UENUM(BlueprintType)
enum class ERiftPlayerAppearanceSlot : uint8
{
	Hair          UMETA(DisplayName="Hair"),
	ArmUpperLeft  UMETA(DisplayName="Arm Upper Left"),
	ArmUpperRight UMETA(DisplayName="Arm Upper Right")
};

USTRUCT(BlueprintType)
struct FRiftPlayerAppearancePartRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	ERiftPlayerAppearanceSlot Slot = ERiftPlayerAppearanceSlot::Hair;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<USkeletalMesh> SkeletalMesh;
};

USTRUCT(BlueprintType)
struct FRiftPlayerAppearanceSelection
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName HairId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ArmUpperLeftId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ArmUpperRightId = NAME_None;

	FName GetPartId(ERiftPlayerAppearanceSlot Slot) const
	{
		switch (Slot)
		{
		case ERiftPlayerAppearanceSlot::Hair:          return HairId;
		case ERiftPlayerAppearanceSlot::ArmUpperLeft:  return ArmUpperLeftId;
		case ERiftPlayerAppearanceSlot::ArmUpperRight: return ArmUpperRightId;
		default:                                       return NAME_None;
		}
	}

	void SetPartId(ERiftPlayerAppearanceSlot Slot, FName PartId)
	{
		switch (Slot)
		{
		case ERiftPlayerAppearanceSlot::Hair:          HairId = PartId;         break;
		case ERiftPlayerAppearanceSlot::ArmUpperLeft:  ArmUpperLeftId = PartId; break;
		case ERiftPlayerAppearanceSlot::ArmUpperRight: ArmUpperRightId = PartId; break;
		default:                                                                  break;
		}
	}
};
