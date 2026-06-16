// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/BaseCharacter.h"

#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"

namespace
{
UStaticMesh* FindWeaponStaticMesh(
	const TMap<ERiftWeaponSlot, TObjectPtr<UStaticMesh>>& Weapons,
	const ERiftWeaponSlot Slot
)
{
	const TObjectPtr<UStaticMesh>* StaticMesh = Weapons.Find(Slot);
	return StaticMesh ? StaticMesh->Get() : nullptr;
}
}

ABaseCharacter::ABaseCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	WeaponHandLeftMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Weapon_HandLeft"));
	WeaponHandLeftMesh->SetupAttachment(GetMesh(), RiftWeapon::GetWeaponSlotSocketName(ERiftWeaponSlot::HandLeft));
	WeaponHandLeftMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	WeaponHandRightMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Weapon_HandRight"));
	WeaponHandRightMesh->SetupAttachment(GetMesh(), RiftWeapon::GetWeaponSlotSocketName(ERiftWeaponSlot::HandRight));
	WeaponHandRightMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	WeaponArmLeftMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Weapon_ArmLeft"));
	WeaponArmLeftMesh->SetupAttachment(GetMesh(), RiftWeapon::GetWeaponSlotSocketName(ERiftWeaponSlot::ArmLeft));
	WeaponArmLeftMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	WeaponArmRightMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Weapon_ArmRight"));
	WeaponArmRightMesh->SetupAttachment(GetMesh(), RiftWeapon::GetWeaponSlotSocketName(ERiftWeaponSlot::ArmRight));
	WeaponArmRightMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	if (GetMesh())
	{
		GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	}
}

void ABaseCharacter::OnDeathVisual_Implementation(const ERiftHitReactDirection Direction)
{
	static_cast<void>(Direction);
}

void ABaseCharacter::ApplyWeapons(const TMap<ERiftWeaponSlot, TObjectPtr<UStaticMesh>>& Weapons)
{
	WeaponHandLeftMesh->SetStaticMesh(FindWeaponStaticMesh(Weapons, ERiftWeaponSlot::HandLeft));
	WeaponHandRightMesh->SetStaticMesh(FindWeaponStaticMesh(Weapons, ERiftWeaponSlot::HandRight));
	WeaponArmLeftMesh->SetStaticMesh(FindWeaponStaticMesh(Weapons, ERiftWeaponSlot::ArmLeft));
	WeaponArmRightMesh->SetStaticMesh(FindWeaponStaticMesh(Weapons, ERiftWeaponSlot::ArmRight));
}

UStaticMeshComponent* ABaseCharacter::GetWeaponMesh(const ERiftWeaponSlot Slot) const
{
	switch (Slot)
	{
	case ERiftWeaponSlot::HandLeft:
		return WeaponHandLeftMesh;
	case ERiftWeaponSlot::HandRight:
		return WeaponHandRightMesh;
	case ERiftWeaponSlot::ArmLeft:
		return WeaponArmLeftMesh;
	case ERiftWeaponSlot::ArmRight:
		return WeaponArmRightMesh;
	default:
		return nullptr;
	}
}
