// Fill out your copyright notice in the Description page of Project Settings.

#include "Combat/RiftWeaponTraceComponent.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystem/Effects/GE_GainResource.h"
#include "AbilitySystem/Effects/GE_MeleeDamage.h"
#include "AbilitySystem/RiftGameplayTags.h"
#include "Character/EnemyCharacter.h"
#include "Character/PlayerCharacter.h"
#include "Combat/RiftCombatFeedbackComponent.h"
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

void URiftWeaponTraceComponent::SetIncomingHitParams(
	const float Damage,
	const float PoiseDamage,
	const float UltimateChargeOnHit
)
{
	IncomingDamage = FMath::Max(0.0f, Damage);
	IncomingPoiseDamage = FMath::Max(0.0f, PoiseDamage);
	IncomingUltimateCharge = FMath::Max(0.0f, UltimateChargeOnHit);
}

void URiftWeaponTraceComponent::SetIncomingCombatCueConfig(URiftCombatCueConfig* CombatCueConfig)
{
	IncomingCombatCueConfig = CombatCueConfig;
}

void URiftWeaponTraceComponent::SetIncomingHitStopConfig(const FRiftMeleeHitStopConfig& HitStopConfig)
{
	IncomingHitStopConfig = HitStopConfig;
}

void URiftWeaponTraceComponent::SetIncomingCameraShake(
	TSubclassOf<UCameraShakeBase> Shake,
	const FVector2D Dir
)
{
	IncomingCameraShake = Shake;
	IncomingCameraShakeDir = Dir;
}

void URiftWeaponTraceComponent::StartHitWindow(const ERiftWeaponSlot Slot)
{
	if (Slot != ERiftWeaponSlot::HandLeft && Slot != ERiftWeaponSlot::HandRight) return;

	AActor* OwnerActor = GetOwner();
	if (!OwnerActor || !OwnerActor->HasAuthority()) return;
	APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(OwnerActor);
	if (!PlayerCharacter) return;

	// v1 uses server-authoritative melee traces only; client-predicted hit cosmetics can be added later.
	int32& HitWindowRefCount = HitWindowRefCounts.FindOrAdd(Slot);
	HitWindowRefCount++;
	if (HitWindowRefCount > 1)
	{
		return;
	}

	HitActorsByTraceId.FindOrAdd(GetWeaponTraceId(Slot)).Empty();
	RemoveTraceCacheForSlot(Slot);

	UStaticMeshComponent* WeaponMesh = PlayerCharacter->GetWeaponMesh(Slot);
	if (!WeaponMesh) return;

	FVector TraceStart = FVector::ZeroVector;
	FVector TraceEnd = FVector::ZeroVector;
	if (!GetWeaponTraceLocations(WeaponMesh, TraceStart, TraceEnd)) return;

	FWeaponTraceCache TraceCache;
	TraceCache.WeaponSlot = Slot;
	TraceCache.WeaponMesh = WeaponMesh;
	TraceCache.TraceRadius = WeaponTraceRadius;
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
		HitActorsByTraceId.Remove(GetWeaponTraceId(Slot));
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
	APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(OwnerActor);
	UWorld* World = GetWorld();
	UStaticMeshComponent* Mesh = TraceCache.WeaponMesh.Get();
	if (!OwnerActor || !World || !Mesh || !PlayerCharacter) return;

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

	for (int32 SampleIndex = 0; SampleIndex < SampleCount; ++SampleIndex)
	{
		const float Alpha = static_cast<float>(SampleIndex) / static_cast<float>(SampleCount - 1);
		const FVector PreviousPoint = FMath::Lerp(TraceCache.PreviousStart, TraceCache.PreviousEnd, Alpha);
		const FVector CurrentPoint = FMath::Lerp(CurrentStart, CurrentEnd, Alpha);

		FRiftMeleeTraceSource WeaponSource;
		WeaponSource.TraceId = GetWeaponTraceId(TraceCache.WeaponSlot);
		WeaponSource.WeaponSlot = TraceCache.WeaponSlot;
		WeaponSource.Start = PreviousPoint;
		WeaponSource.End = CurrentPoint;
		WeaponSource.Radius = TraceRadius;
		WeaponSource.CombatCueConfig = IncomingCombatCueConfig;
		WeaponSource.HitStopConfig = IncomingHitStopConfig;
		WeaponSource.bPlayHitStop = true;
		WeaponSource.bPlayCameraShake = true;
		ProcessMeleeTraceSource(WeaponSource);
	}

	TraceCache.PreviousStart = CurrentStart;
	TraceCache.PreviousEnd = CurrentEnd;
}

void URiftWeaponTraceComponent::ProcessMeleeTraceSource(const FRiftMeleeTraceSource& Source)
{
	AActor* OwnerActor = GetOwner();
	UWorld* World = GetWorld();
	if (!OwnerActor || !World || Source.TraceId == NAME_None || Source.Radius <= 0.0f)
	{
		return;
	}

	TArray<FHitResult> HitResults;
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(RiftMeleeTraceSource), false, OwnerActor);
	QueryParams.AddIgnoredActor(OwnerActor);

	World->SweepMultiByChannel(
		HitResults,
		Source.Start,
		Source.End,
		FQuat::Identity,
		ECC_Pawn,
		FCollisionShape::MakeSphere(Source.Radius),
		QueryParams
	);

	for (const FHitResult& Hit : HitResults)
	{
		ProcessMeleeTraceHit(Source, Hit);
	}

	if (RiftDebugCVars::IsCombatDebugEnabled())
	{
		const FColor DebugColor = Source.bPlayCameraShake ? FColor::Yellow : FColor::Cyan;
		DrawDebugLine(World, Source.Start, Source.End, DebugColor, false, 0.1f, 0, 0.75f);
		DrawDebugSphere(World, Source.End, Source.Radius, 12, DebugColor, false, 0.1f, 0, 0.75f);
	}
}

void URiftWeaponTraceComponent::ProcessMeleeTraceHit(
	const FRiftMeleeTraceSource& Source,
	const FHitResult& Hit
)
{
	APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(GetOwner());
	AEnemyCharacter* EnemyCharacter = Cast<AEnemyCharacter>(Hit.GetActor());
	if (!PlayerCharacter || !EnemyCharacter) return;

	const float Damage = IncomingDamage * Source.DamageMultiplier;
	const float PoiseDamage = IncomingPoiseDamage * Source.PoiseDamageMultiplier;
	const float UltimateCharge = IncomingUltimateCharge * Source.UltimateChargeMultiplier;
	if (Damage <= 0.0f && PoiseDamage <= 0.0f && UltimateCharge <= 0.0f)
	{
		return;
	}

	const TObjectKey<AActor> HitActorKey(EnemyCharacter);
	TSet<TObjectKey<AActor>>& HitActorsThisWindow = HitActorsByTraceId.FindOrAdd(Source.TraceId);
	if (HitActorsThisWindow.Contains(HitActorKey)) return;

	UAbilitySystemComponent* SourceAbilitySystemComponent = PlayerCharacter->GetAbilitySystemComponent();
	UAbilitySystemComponent* TargetAbilitySystemComponent = EnemyCharacter->GetAbilitySystemComponent();
	if (!SourceAbilitySystemComponent || !TargetAbilitySystemComponent) return;

	if (TargetAbilitySystemComponent->HasMatchingGameplayTag(RiftGameplayTags::State_Enemy_Intro))
	{
		HitActorsThisWindow.Add(HitActorKey);
		return;
	}

	FGameplayEffectContextHandle EffectContext = SourceAbilitySystemComponent->MakeEffectContext();
	EffectContext.AddInstigator(PlayerCharacter, PlayerCharacter);
	EffectContext.AddSourceObject(PlayerCharacter);

	FGameplayEffectSpecHandle DamageSpecHandle = SourceAbilitySystemComponent->MakeOutgoingSpec(
		UGE_MeleeDamage::StaticClass(),
		1.0f,
		EffectContext
	);
	if (!DamageSpecHandle.IsValid()) return;

	HitActorsThisWindow.Add(HitActorKey);

	DamageSpecHandle.Data->SetSetByCallerMagnitude(RiftGameplayTags::SetByCaller_Damage, Damage);
	DamageSpecHandle.Data->SetSetByCallerMagnitude(RiftGameplayTags::SetByCaller_PoiseDamage, PoiseDamage);
	SourceAbilitySystemComponent->ApplyGameplayEffectSpecToTarget(
		*DamageSpecHandle.Data.Get(),
		TargetAbilitySystemComponent
	);

	if (UltimateCharge > 0.0f)
	{
		FGameplayEffectSpecHandle GainSpecHandle = SourceAbilitySystemComponent->MakeOutgoingSpec(
			UGE_GainResource::StaticClass(),
			1.0f,
			EffectContext
		);
		if (GainSpecHandle.IsValid())
		{
			GainSpecHandle.Data->SetSetByCallerMagnitude(
				RiftGameplayTags::SetByCaller_UltimateCharge,
				UltimateCharge
			);
			SourceAbilitySystemComponent->ApplyGameplayEffectSpecToTarget(
				*GainSpecHandle.Data.Get(),
				SourceAbilitySystemComponent
			);
		}
	}

	const FVector ImpactLocation = Hit.ImpactPoint.IsNearlyZero() ? Hit.Location : Hit.ImpactPoint;
	const FVector ImpactNormal = Hit.ImpactNormal.GetSafeNormal();
	if (Source.CombatCueConfig)
	{
		EnemyCharacter->Multicast_PlayCombatImpact(
			Source.CombatCueConfig,
			ImpactLocation,
			ImpactNormal,
			EnemyCharacter->IsBlocking()
		);
	}

	if ((Source.bPlayHitStop && Source.HitStopConfig.bEnableHitStop) || Source.bPlayCameraShake)
	{
		if (URiftCombatFeedbackComponent* FeedbackComponent = PlayerCharacter->GetCombatFeedbackComponent())
		{
			FeedbackComponent->Multicast_PlayMeleeHitFeedback(
				EnemyCharacter,
				Source.bPlayHitStop,
				Source.HitStopConfig,
				Source.bPlayCameraShake,
				IncomingCameraShake,
				IncomingCameraShakeDir
			);
		}
	}

	if (RiftDebugCVars::IsCombatDebugEnabled())
	{
		if (UWorld* World = GetWorld())
		{
			const FColor DebugColor = Source.bPlayCameraShake ? FColor::Yellow : FColor::Cyan;
			DrawDebugSphere(World, ImpactLocation, Source.Radius * 1.25f, 12, DebugColor, false, 0.2f, 0, 2.0f);
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

FName URiftWeaponTraceComponent::GetWeaponTraceId(const ERiftWeaponSlot Slot)
{
	switch (Slot)
	{
	case ERiftWeaponSlot::HandLeft:
		return TEXT("Weapon_Left");
	case ERiftWeaponSlot::HandRight:
		return TEXT("Weapon_Right");
	default:
		return NAME_None;
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
