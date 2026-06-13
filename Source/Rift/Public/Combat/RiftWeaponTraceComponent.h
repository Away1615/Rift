// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RiftWeaponTraceComponent.generated.h"

class UStaticMeshComponent;

UENUM(BlueprintType)
enum class ERiftWeaponSlot : uint8
{
	Left UMETA(DisplayName="Left"),
	Right UMETA(DisplayName="Right")
};

UENUM(BlueprintType)
enum class ERiftWeaponTraceSocket : uint8
{
	TraceStart UMETA(DisplayName="Trace Start"),
	TraceEnd UMETA(DisplayName="Trace End")
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class RIFT_API URiftWeaponTraceComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	URiftWeaponTraceComponent();

	void SetIncomingHitParams(float Damage);
	void StartHitWindow(ERiftWeaponSlot Slot);
	void EndHitWindow(ERiftWeaponSlot Slot);

protected:
	virtual void TickComponent(
		float DeltaTime,
		ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction
	) override;

private:
	struct FWeaponTraceCache
	{
		ERiftWeaponSlot WeaponSlot = ERiftWeaponSlot::Left;
		TWeakObjectPtr<UStaticMeshComponent> WeaponMesh;
		float TraceRadius = 15.0f;
		FVector PreviousStart = FVector::ZeroVector;
		FVector PreviousEnd = FVector::ZeroVector;
	};

	bool GetWeaponTraceLocations(const UStaticMeshComponent* MeshComp, FVector& OutStart, FVector& OutEnd) const;
	void TraceWeapon(FWeaponTraceCache& TraceCache);
	void ProcessHit(ERiftWeaponSlot Slot, float TraceRadius, const FHitResult& Hit);
	void RemoveTraceCacheForSlot(ERiftWeaponSlot Slot);
	static FName GetSocketNameForSlot(ERiftWeaponTraceSocket Socket);

	float IncomingDamage = 0.0f;
	TMap<ERiftWeaponSlot, int32> HitWindowRefCounts;
	TArray<FWeaponTraceCache> WeaponTraceCaches;
	TMap<ERiftWeaponSlot, TSet<TObjectKey<AActor>>> HitActorsBySlot;
};
