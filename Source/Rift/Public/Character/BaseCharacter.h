// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Combat/RiftHitReactionTypes.h"
#include "Combat/RiftWeaponTypes.h"
#include "GameFramework/Character.h"
#include "BaseCharacter.generated.h"

class UStaticMesh;
class UStaticMeshComponent;

UCLASS(Abstract)
class RIFT_API ABaseCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ABaseCharacter();

	void ApplyWeapons(const TMap<ERiftWeaponSlot, TObjectPtr<UStaticMesh>>& Weapons);
	UStaticMeshComponent* GetWeaponMesh(ERiftWeaponSlot Slot) const;

	UFUNCTION(BlueprintNativeEvent, Category="Death")
	void OnDeathVisual(ERiftHitReactDirection Direction);
	virtual void OnDeathVisual_Implementation(ERiftHitReactDirection Direction);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Weapon")
	TObjectPtr<UStaticMeshComponent> WeaponHandLeftMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Weapon")
	TObjectPtr<UStaticMeshComponent> WeaponHandRightMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Weapon")
	TObjectPtr<UStaticMeshComponent> WeaponArmLeftMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Weapon")
	TObjectPtr<UStaticMeshComponent> WeaponArmRightMesh;
};
