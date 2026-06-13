// Fill out your copyright notice in the Description page of Project Settings.

#include "Combat/RiftWeaponTraceComponent.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystem/Effects/GE_MeleeDamage.h"
#include "AbilitySystem/RiftGameplayTags.h"
#include "Character/EnemyCharacter.h"
#include "Character/PlayerCharacter.h"
#include "Components/StaticMeshComponent.h"
#include "Debug/RiftDebugCVars.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "GameFramework/PlayerState.h"

URiftWeaponTraceComponent::URiftWeaponTraceComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	SetComponentTickEnabled(false);
}

void URiftWeaponTraceComponent::SetIncomingHitParams(const float Damage, const float PoiseDamage)
{
	IncomingDamage = FMath::Max(0.0f, Damage);
	IncomingPoiseDamage = FMath::Max(0.0f, PoiseDamage);
}

void URiftWeaponTraceComponent::StartHitWindow(const ERiftWeaponSlot Slot)
{
	AActor* OwnerActor = GetOwner();
	if (!OwnerActor || !OwnerActor->HasAuthority()) return;

	// v1 uses server-authoritative melee traces only; client-predicted hit cosmetics can be added later.
	int32& HitWindowRefCount = HitWindowRefCounts.FindOrAdd(Slot);
	HitWindowRefCount++;
	if (HitWindowRefCount > 1)
	{
		return;
	}

	HitActorsBySlot.FindOrAdd(Slot).Empty();
	RemoveTraceCacheForSlot(Slot);

	const APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(OwnerActor);
	if (!PlayerCharacter) return;

	UStaticMeshComponent* WeaponMesh = PlayerCharacter->GetWeaponMeshComponent(Slot);
	if (!WeaponMesh) return;

	FVector TraceStart = FVector::ZeroVector;
	FVector TraceEnd = FVector::ZeroVector;
	if (!GetWeaponTraceLocations(WeaponMesh, TraceStart, TraceEnd)) return;

	FWeaponTraceCache TraceCache;
	TraceCache.WeaponSlot = Slot;
	TraceCache.WeaponMesh = WeaponMesh;
	TraceCache.TraceRadius = PlayerCharacter->GetWeaponTraceRadius(Slot);
	TraceCache.PreviousStart = TraceStart;
	TraceCache.PreviousEnd = TraceEnd;
	WeaponTraceCaches.Add(TraceCache);

	SetComponentTickEnabled(WeaponTraceCaches.Num() > 0);
}

void URiftWeaponTraceComponent::EndHitWindow(const ERiftWeaponSlot Slot)
{
	int32* HitWindowRefCount = HitWindowRefCounts.Find(Slot);
	if (!HitWindowRefCount) return;

	*HitWindowRefCount = FMath::Max(0, *HitWindowRefCount - 1);
	if (*HitWindowRefCount == 0)
	{
		HitWindowRefCounts.Remove(Slot);
		HitActorsBySlot.Remove(Slot);
		RemoveTraceCacheForSlot(Slot);
	}

	SetComponentTickEnabled(WeaponTraceCaches.Num() > 0);
}

void URiftWeaponTraceComponent::TickComponent(
	const float DeltaTime,
	const ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction
)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	AActor* OwnerActor = GetOwner();
	if (!OwnerActor || !OwnerActor->HasAuthority()) return;

	for (FWeaponTraceCache& TraceCache : WeaponTraceCaches)
	{
		TraceWeapon(TraceCache);
	}
}

bool URiftWeaponTraceComponent::GetWeaponTraceLocations(
	const UStaticMeshComponent* MeshComp,
	FVector& OutStart,
	FVector& OutEnd
) const
{
	if (!MeshComp) return false;

	const FName StartSocketName = GetSocketNameForSlot(ERiftWeaponTraceSocket::TraceStart);
	const FName EndSocketName = GetSocketNameForSlot(ERiftWeaponTraceSocket::TraceEnd);
	if (StartSocketName.IsNone() || EndSocketName.IsNone()) return false;
	if (!MeshComp->DoesSocketExist(StartSocketName) || !MeshComp->DoesSocketExist(EndSocketName)) return false;

	OutStart = MeshComp->GetSocketLocation(StartSocketName);
	OutEnd = MeshComp->GetSocketLocation(EndSocketName);
	return true;
}

void URiftWeaponTraceComponent::TraceWeapon(FWeaponTraceCache& TraceCache)
{
	AActor* OwnerActor = GetOwner();
	UWorld* World = GetWorld();
	UStaticMeshComponent* Mesh = TraceCache.WeaponMesh.Get();
	if (!OwnerActor || !World || !Mesh) return;

	FVector CurrentStart = FVector::ZeroVector;
	FVector CurrentEnd = FVector::ZeroVector;
	if (!GetWeaponTraceLocations(Mesh, CurrentStart, CurrentEnd)) return;

	const float TraceRadius = FMath::Max(0.0f, TraceCache.TraceRadius);
	if (TraceRadius <= 0.0f)
	{
		TraceCache.PreviousStart = CurrentStart;
		TraceCache.PreviousEnd = CurrentEnd;
		return;
	}

	constexpr int32 SampleCount = 3;
	const FCollisionShape TraceShape = FCollisionShape::MakeSphere(TraceRadius);
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(RiftWeaponTrace), false, OwnerActor);
	QueryParams.AddIgnoredActor(OwnerActor);

	const bool bDebug = RiftDebugCVars::IsCombatDebugEnabled();

	for (int32 SampleIndex = 0; SampleIndex < SampleCount; ++SampleIndex)
	{
		const float Alpha = static_cast<float>(SampleIndex) / static_cast<float>(SampleCount - 1);
		const FVector PreviousPoint = FMath::Lerp(TraceCache.PreviousStart, TraceCache.PreviousEnd, Alpha);
		const FVector CurrentPoint = FMath::Lerp(CurrentStart, CurrentEnd, Alpha);

		TArray<FHitResult> HitResults;
		World->SweepMultiByChannel(
			HitResults,
			PreviousPoint,
			CurrentPoint,
			FQuat::Identity,
			ECC_Pawn,
			TraceShape,
			QueryParams
		);

		bool bHitEnemy = false;
		for (const FHitResult& Hit : HitResults)
		{
			if (Cast<AEnemyCharacter>(Hit.GetActor()))
			{
				bHitEnemy = true;
				break;
			}
		}

		if (bDebug)
		{
			const FColor DebugColor = bHitEnemy ? FColor::Yellow : FColor::Green;
			DrawDebugLine(World, PreviousPoint, CurrentPoint, DebugColor, false, 0.1f, 0, 0.75f);
			DrawDebugSphere(World, CurrentPoint, TraceRadius, 12, DebugColor, false, 0.1f, 0, 0.75f);
		}

		for (const FHitResult& Hit : HitResults)
		{
			ProcessHit(TraceCache.WeaponSlot, TraceCache.TraceRadius, Hit);
		}
	}

	TraceCache.PreviousStart = CurrentStart;
	TraceCache.PreviousEnd = CurrentEnd;
}

void URiftWeaponTraceComponent::ProcessHit(const ERiftWeaponSlot Slot, const float TraceRadius, const FHitResult& Hit)
{
	APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(GetOwner());
	AEnemyCharacter* EnemyCharacter = Cast<AEnemyCharacter>(Hit.GetActor());
	if (!PlayerCharacter || !EnemyCharacter) return;

	const TObjectKey<AActor> HitActorKey(EnemyCharacter);
	TSet<TObjectKey<AActor>>& HitActorsThisWindow = HitActorsBySlot.FindOrAdd(Slot);
	if (HitActorsThisWindow.Contains(HitActorKey)) return;

	UAbilitySystemComponent* SourceAbilitySystemComponent = PlayerCharacter->GetAbilitySystemComponent();
	UAbilitySystemComponent* TargetAbilitySystemComponent = EnemyCharacter->GetAbilitySystemComponent();
	if (!SourceAbilitySystemComponent || !TargetAbilitySystemComponent) return;

	FGameplayEffectContextHandle EffectContext = SourceAbilitySystemComponent->MakeEffectContext();
	EffectContext.AddInstigator(PlayerCharacter, PlayerCharacter);
	EffectContext.AddSourceObject(PlayerCharacter);

	FGameplayEffectSpecHandle DamageSpecHandle = SourceAbilitySystemComponent->MakeOutgoingSpec(
		UGE_MeleeDamage::StaticClass(),
		1.0f,
		EffectContext
	);
	if (!DamageSpecHandle.IsValid()) return;

	// Mark as hit only after all required objects/specs are valid.
	HitActorsThisWindow.Add(HitActorKey);

	DamageSpecHandle.Data->SetSetByCallerMagnitude(RiftGameplayTags::SetByCaller_Damage, IncomingDamage);
	DamageSpecHandle.Data->SetSetByCallerMagnitude(RiftGameplayTags::SetByCaller_PoiseDamage, IncomingPoiseDamage);
	SourceAbilitySystemComponent->ApplyGameplayEffectSpecToTarget(
		*DamageSpecHandle.Data.Get(),
		TargetAbilitySystemComponent
	);

	FGameplayCueParameters CueParameters;
	CueParameters.Location = Hit.ImpactPoint.IsNearlyZero() ? Hit.Location : Hit.ImpactPoint;
	CueParameters.Normal = Hit.ImpactNormal.GetSafeNormal();
	CueParameters.Instigator = PlayerCharacter;
	CueParameters.EffectCauser = PlayerCharacter;
	TargetAbilitySystemComponent->ExecuteGameplayCue(RiftGameplayTags::GameplayCue_Combat_MeleeHit, CueParameters);
	PlayerCharacter->Multicast_PlayMeleeHitStop(EnemyCharacter);

	if (RiftDebugCVars::IsCombatDebugEnabled())
	{
		if (UWorld* World = GetWorld())
		{
			DrawDebugSphere(World, CueParameters.Location, TraceRadius * 1.25f, 12, FColor::Yellow, false, 0.2f, 0, 2.0f);
		}
	}
}

void URiftWeaponTraceComponent::RemoveTraceCacheForSlot(const ERiftWeaponSlot Slot)
{
	for (int32 Index = WeaponTraceCaches.Num() - 1; Index >= 0; --Index)
	{
		if (WeaponTraceCaches[Index].WeaponSlot == Slot)
		{
			WeaponTraceCaches.RemoveAtSwap(Index);
		}
	}
}

FName URiftWeaponTraceComponent::GetSocketNameForSlot(const ERiftWeaponTraceSocket Socket)
{
	switch (Socket)
	{
	case ERiftWeaponTraceSocket::TraceStart:
		return TEXT("TraceStart");
	case ERiftWeaponTraceSocket::TraceEnd:
		return TEXT("TraceEnd");
	default:
		return NAME_None;
	}
}
