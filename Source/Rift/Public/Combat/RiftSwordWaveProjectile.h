#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RiftSwordWaveProjectile.generated.h"

class APlayerCharacter;
class UProjectileMovementComponent;
class URiftCombatCueConfig;
class USphereComponent;

UCLASS()
class RIFT_API ARiftSwordWaveProjectile : public AActor
{
	GENERATED_BODY()

public:
	ARiftSwordWaveProjectile();

	void InitializeSwordWave(
		APlayerCharacter* InSourceCharacter,
		float InDamage,
		float InPoiseDamage,
		float InUltimateChargeOnHit,
		float InSpeed,
		float InLifeSeconds,
		URiftCombatCueConfig* InCombatCueConfig
	);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Projectile")
	TObjectPtr<USphereComponent> CollisionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Projectile")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovementComponent;

private:
	UFUNCTION()
	void HandleCollisionOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComponent,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	UFUNCTION()
	void HandleCollisionHit(
		UPrimitiveComponent* HitComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComponent,
		FVector NormalImpulse,
		const FHitResult& Hit
	);

	void ProcessHitActor(AActor* HitActor, const FHitResult& Hit);
	void ApplyDamageToEnemy(class AEnemyCharacter* EnemyCharacter, const FHitResult& Hit);

	UPROPERTY()
	TObjectPtr<APlayerCharacter> SourceCharacter;

	UPROPERTY()
	TObjectPtr<URiftCombatCueConfig> CombatCueConfig;

	float Damage = 0.0f;
	float PoiseDamage = 0.0f;
	float UltimateChargeOnHit = 0.0f;
	bool bHasProcessedHit = false;
};
