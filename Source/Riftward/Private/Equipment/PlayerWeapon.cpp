// Fill out your copyright notice in the Description page of Project Settings.


#include "Equipment/PlayerWeapon.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"


// Sets default values
APlayerWeapon::APlayerWeapon()
{
	bReplicates = true;
	AActor::SetReplicateMovement(true);

	GripRoot = CreateDefaultSubobject<USceneComponent>(TEXT("GripRoot"));
	SetRootComponent(GripRoot);

	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetupAttachment(GripRoot);

	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh->SetGenerateOverlapEvents(false);
}

// Called when the game starts or when spawned
void APlayerWeapon::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void APlayerWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
