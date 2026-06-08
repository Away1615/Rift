// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PlayerWeapon.generated.h"

class UStaticMeshComponent;
class USceneComponent;

UCLASS()
class RIFT_API APlayerWeapon : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	APlayerWeapon();

	USceneComponent* GetGripRoot() const { return GripRoot; }
	UStaticMeshComponent* GetWeaponMesh() const { return WeaponMesh; };
	FVector GetTraceStartLocation() const;
	FVector GetTraceEndLocation() const;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Weapon")
	TObjectPtr<USceneComponent> GripRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Weapon")
	TObjectPtr<UStaticMeshComponent> WeaponMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Weapon|Trace")
	TObjectPtr<USceneComponent> TraceStart;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Weapon|Trace")
	TObjectPtr<USceneComponent> TraceEnd;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
