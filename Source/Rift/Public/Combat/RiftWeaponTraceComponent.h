// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Combat/RiftCombatFeedbackTypes.h"
#include "Combat/RiftWeaponTypes.h"
#include "Components/ActorComponent.h"
#include "RiftWeaponTraceComponent.generated.h"

class UCameraShakeBase;
class URiftCombatCueConfig;
class UStaticMeshComponent;

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

	void SetIncomingHitParams(float Damage, float PoiseDamage, float UltimateChargeOnHit);
	void SetIncomingCombatCueConfig(URiftCombatCueConfig* CombatCueConfig);
	void SetIncomingHitStopConfig(const FRiftMeleeHitStopConfig& HitStopConfig);
	void SetIncomingCameraShake(TSubclassOf<UCameraShakeBase> Shake, FVector2D Dir);
	void StartHitWindow(ERiftWeaponSlot Slot);
	void EndHitWindow(ERiftWeaponSlot Slot);

	UPROPERTY(EditDefaultsOnly, Category="Trace")
	float WeaponTraceRadius = 15.0f;

protected:
	virtual void TickComponent(
		float DeltaTime,
		ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction
	) override;

private:
	struct FWeaponTraceCache
	{
		ERiftWeaponSlot WeaponSlot = ERiftWeaponSlot::HandLeft;
		TWeakObjectPtr<UStaticMeshComponent> WeaponMesh;
		float TraceRadius = 15.0f;
		FVector PreviousStart = FVector::ZeroVector;
		FVector PreviousEnd = FVector::ZeroVector;
	};

	struct FRiftMeleeTraceSource
	{
		FName TraceId = NAME_None;
		ERiftWeaponSlot WeaponSlot = ERiftWeaponSlot::HandLeft;
		FVector Start = FVector::ZeroVector;
		FVector End = FVector::ZeroVector;
		float Radius = 0.0f;
		float DamageMultiplier = 1.0f;
		float PoiseDamageMultiplier = 1.0f;
		float UltimateChargeMultiplier = 1.0f;
		URiftCombatCueConfig* CombatCueConfig = nullptr;
		FRiftMeleeHitStopConfig HitStopConfig;
		bool bPlayHitStop = true;
		bool bPlayCameraShake = true;
	};

	bool GetWeaponTraceLocations(const UStaticMeshComponent* MeshComp, FVector& OutStart, FVector& OutEnd) const;
	void TraceWeapon(FWeaponTraceCache& TraceCache);
	void ProcessMeleeTraceSource(const FRiftMeleeTraceSource& Source);
	void ProcessMeleeTraceHit(const FRiftMeleeTraceSource& Source, const FHitResult& Hit);
	void RemoveTraceCacheForSlot(ERiftWeaponSlot Slot);
	static FName GetSocketNameForSlot(ERiftWeaponTraceSocket Socket);
	static FName GetWeaponTraceId(ERiftWeaponSlot Slot);

	float IncomingDamage = 0.0f;
	float IncomingPoiseDamage = 0.0f;
	float IncomingUltimateCharge = 0.0f;
	TObjectPtr<URiftCombatCueConfig> IncomingCombatCueConfig;
	FRiftMeleeHitStopConfig IncomingHitStopConfig;
	TSubclassOf<UCameraShakeBase> IncomingCameraShake;
	FVector2D IncomingCameraShakeDir = FVector2D(1.0f, 0.0f);
	TMap<ERiftWeaponSlot, int32> HitWindowRefCounts;
	TArray<FWeaponTraceCache> WeaponTraceCaches;
	TMap<FName, TSet<TObjectKey<AActor>>> HitActorsByTraceId;
};
