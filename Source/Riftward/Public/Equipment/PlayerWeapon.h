// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PlayerWeapon.generated.h"

class UStaticMeshComponent;
class USceneComponent;

UCLASS()
class RIFTWARD_API APlayerWeapon : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	APlayerWeapon();

	USceneComponent* GetGripRoot() const { return GripRoot; }
	UStaticMeshComponent* GetWeaponMesh() const { return WeaponMesh; };

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Weapon")
	TObjectPtr<USceneComponent> GripRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Weapon")
	TObjectPtr<UStaticMeshComponent> WeaponMesh;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
