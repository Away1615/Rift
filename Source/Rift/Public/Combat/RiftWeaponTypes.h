#pragma once

#include "CoreMinimal.h"
#include "RiftWeaponTypes.generated.h"

UENUM(BlueprintType)
enum class ERiftWeaponSlot : uint8
{
	HandLeft UMETA(DisplayName="Hand Left"),
	HandRight UMETA(DisplayName="Hand Right"),
	ArmLeft UMETA(DisplayName="Arm Left"),
	ArmRight UMETA(DisplayName="Arm Right")
};

namespace RiftWeapon
{
	inline FName GetWeaponSlotSocketName(const ERiftWeaponSlot Slot)
	{
		switch (Slot)
		{
		case ERiftWeaponSlot::HandLeft:
			return FName(TEXT("Weapon_Hand_L"));
		case ERiftWeaponSlot::HandRight:
			return FName(TEXT("Weapon_Hand_R"));
		case ERiftWeaponSlot::ArmLeft:
			return FName(TEXT("Weapon_Arm_L"));
		case ERiftWeaponSlot::ArmRight:
			return FName(TEXT("Weapon_Arm_R"));
		default:
			return NAME_None;
		}
	}
}
