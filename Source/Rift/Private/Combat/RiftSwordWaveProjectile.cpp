#include "Combat/RiftSwordWaveProjectile.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystem/Effects/GE_GainResource.h"
#include "AbilitySystem/Effects/GE_MeleeDamage.h"
#include "AbilitySystem/RiftGameplayTags.h"
#include "Character/EnemyCharacter.h"
#include "Character/PlayerCharacter.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

ARiftSwordWaveProjectile::ARiftSwordWaveProjectile()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(true);

	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	SetRootComponent(CollisionComponent);
	CollisionComponent->InitSphereRadius(24.0f);
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionComponent->SetCollisionObjectType(ECC_WorldDynamic);
	CollisionComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	CollisionComponent->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	CollisionComponent->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
	CollisionComponent->SetGenerateOverlapEvents(true);
	CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &ARiftSwordWaveProjectile::HandleCollisionOverlap);
	CollisionComponent->OnComponentHit.AddDynamic(this, &ARiftSwordWaveProjectile::HandleCollisionHit);

	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	ProjectileMovementComponent->InitialSpeed = 1800.0f;
	ProjectileMovementComponent->MaxSpeed = 1800.0f;
	ProjectileMovementComponent->ProjectileGravityScale = 0.0f;
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->bInitialVelocityInLocalSpace = true;
}

void ARiftSwordWaveProjectile::InitializeSwordWave(
	APlayerCharacter* InSourceCharacter,
	const float InDamage,
	const float InPoiseDamage,
	const float InUltimateChargeOnHit,
	const float InSpeed,
	const float InLifeSeconds,
	URiftCombatCueConfig* InCombatCueConfig
)
{
	SourceCharacter = InSourceCharacter;
	Damage = FMath::Max(0.0f, InDamage);
	PoiseDamage = FMath::Max(0.0f, InPoiseDamage);
	UltimateChargeOnHit = FMath::Max(0.0f, InUltimateChargeOnHit);
	CombatCueConfig = InCombatCueConfig;

	const float ClampedSpeed = FMath::Max(0.0f, InSpeed);
	if (ProjectileMovementComponent)
	{
		ProjectileMovementComponent->InitialSpeed = ClampedSpeed;
		ProjectileMovementComponent->MaxSpeed = ClampedSpeed;
		ProjectileMovementComponent->Velocity = GetActorForwardVector() * ClampedSpeed;
	}

	SetLifeSpan(FMath::Max(0.1f, InLifeSeconds));
}

void ARiftSwordWaveProjectile::HandleCollisionOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComponent,
	const int32 OtherBodyIndex,
	const bool bFromSweep,
	const FHitResult& SweepResult
)
{
	static_cast<void>(OverlappedComponent);
	static_cast<void>(OtherComponent);
	static_cast<void>(OtherBodyIndex);
	static_cast<void>(bFromSweep);

	if (!HasAuthority()) return;

	ProcessHitActor(OtherActor, SweepResult);
}

void ARiftSwordWaveProjectile::HandleCollisionHit(
	UPrimitiveComponent* HitComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComponent,
	FVector NormalImpulse,
	const FHitResult& Hit
)
{
	static_cast<void>(HitComponent);
	static_cast<void>(OtherComponent);
	static_cast<void>(NormalImpulse);

	if (!HasAuthority()) return;

	ProcessHitActor(OtherActor, Hit);
}

void ARiftSwordWaveProjectile::ProcessHitActor(AActor* HitActor, const FHitResult& Hit)
{
	if (bHasProcessedHit || !HitActor || HitActor == SourceCharacter)
	{
		return;
	}

	if (Cast<APlayerCharacter>(HitActor))
	{
		return;
	}

	if (AEnemyCharacter* EnemyCharacter = Cast<AEnemyCharacter>(HitActor))
	{
		bHasProcessedHit = true;
		ApplyDamageToEnemy(EnemyCharacter, Hit);
		Destroy();
		return;
	}

	bHasProcessedHit = true;
	Destroy();
}

void ARiftSwordWaveProjectile::ApplyDamageToEnemy(AEnemyCharacter* EnemyCharacter, const FHitResult& Hit)
{
	if (!EnemyCharacter || !SourceCharacter)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("SwordWaveProjectile damage failed: invalid source or enemy. Projectile=%s Source=%s Enemy=%s"),
			*GetNameSafe(this),
			*GetNameSafe(SourceCharacter),
			*GetNameSafe(EnemyCharacter)
		);
		return;
	}

	UAbilitySystemComponent* SourceAbilitySystemComponent = SourceCharacter->GetAbilitySystemComponent();
	UAbilitySystemComponent* TargetAbilitySystemComponent = EnemyCharacter->GetAbilitySystemComponent();
	if (!SourceAbilitySystemComponent || !TargetAbilitySystemComponent)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("SwordWaveProjectile damage failed: missing ASC. Projectile=%s Source=%s SourceASC=%s Enemy=%s TargetASC=%s"),
			*GetNameSafe(this),
			*GetNameSafe(SourceCharacter),
			*GetNameSafe(SourceAbilitySystemComponent),
			*GetNameSafe(EnemyCharacter),
			*GetNameSafe(TargetAbilitySystemComponent)
		);
		return;
	}

	const bool bWasBlocked = EnemyCharacter->IsBlocking();
	const FVector ImpactPoint = FVector(Hit.ImpactPoint);
	const FVector ImpactLocation = ImpactPoint.IsNearlyZero() ? GetActorLocation() : ImpactPoint;
	FVector ImpactNormal = Hit.ImpactNormal.GetSafeNormal();
	if (ImpactNormal.IsNearlyZero())
	{
		ImpactNormal = -GetActorForwardVector();
	}

	FGameplayEffectContextHandle EffectContext = SourceAbilitySystemComponent->MakeEffectContext();
	EffectContext.AddInstigator(SourceCharacter, SourceCharacter);
	EffectContext.AddSourceObject(this);

	FGameplayEffectSpecHandle DamageSpecHandle = SourceAbilitySystemComponent->MakeOutgoingSpec(
		UGE_MeleeDamage::StaticClass(),
		1.0f,
		EffectContext
	);
	if (!DamageSpecHandle.IsValid())
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("SwordWaveProjectile damage failed: DamageSpec invalid. Projectile=%s Source=%s Enemy=%s"),
			*GetNameSafe(this),
			*GetNameSafe(SourceCharacter),
			*GetNameSafe(EnemyCharacter)
		);
		return;
	}

	DamageSpecHandle.Data->SetSetByCallerMagnitude(RiftGameplayTags::SetByCaller_Damage, Damage);
	DamageSpecHandle.Data->SetSetByCallerMagnitude(RiftGameplayTags::SetByCaller_PoiseDamage, PoiseDamage);
	SourceAbilitySystemComponent->ApplyGameplayEffectSpecToTarget(
		*DamageSpecHandle.Data.Get(),
		TargetAbilitySystemComponent
	);

	if (UltimateChargeOnHit > 0.0f)
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
				UltimateChargeOnHit
			);
			SourceAbilitySystemComponent->ApplyGameplayEffectSpecToTarget(
				*GainSpecHandle.Data.Get(),
				SourceAbilitySystemComponent
			);
		}
	}

	if (CombatCueConfig)
	{
		EnemyCharacter->Multicast_PlayCombatImpact(
			CombatCueConfig,
			ImpactLocation,
			ImpactNormal,
			bWasBlocked
		);
	}
}
