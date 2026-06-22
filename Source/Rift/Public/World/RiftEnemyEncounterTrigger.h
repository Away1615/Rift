#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RiftEnemyEncounterTrigger.generated.h"

class AEnemyCharacter;
class APlayerCharacter;
class UBoxComponent;
class UEnemyCharacterConfig;

USTRUCT(BlueprintType)
struct FRiftEnemyEncounterSpawnEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Enemy")
	TSubclassOf<AEnemyCharacter> EnemyClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Enemy")
	TObjectPtr<UEnemyCharacterConfig> EnemyConfig;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Enemy", meta=(ClampMin="1"))
	int32 SpawnCost = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Enemy", meta=(ClampMin="0.0"))
	float SpawnWeight = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Enemy", meta=(ClampMin="0"))
	int32 MaxSpawnCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Enemy", meta=(ClampMin="0.0"))
	float SpawnRadius = 600.0f;
};

UCLASS(Blueprintable)
class RIFT_API ARiftEnemyEncounterTrigger : public AActor
{
	GENERATED_BODY()

public:
	ARiftEnemyEncounterTrigger();

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category="Rift|Encounter")
	void TriggerEncounter();

	UFUNCTION(BlueprintPure, Category="Rift|Encounter")
	bool HasTriggered() const { return bHasTriggered; }

	UFUNCTION(BlueprintImplementableEvent, Category="Rift|Encounter")
	void OnEncounterTriggered();

	UFUNCTION(BlueprintImplementableEvent, Category="Rift|Encounter")
	void OnEncounterEnemySpawned(AEnemyCharacter* SpawnedEnemy);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Rift|Encounter")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Rift|Encounter")
	TObjectPtr<UBoxComponent> TriggerBox;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Rift|Encounter")
	bool bTriggerOnce = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Rift|Encounter", meta=(ClampMin="0"))
	int32 SpawnBudget = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Rift|Encounter")
	TArray<FRiftEnemyEncounterSpawnEntry> EnemyPool;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Rift|Encounter")
	TArray<TObjectPtr<AActor>> SpawnPoints;

private:
	UFUNCTION()
	void HandleTriggerBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	void SpawnEncounterEnemies();
	bool SpawnEnemyFromEntry(const FRiftEnemyEncounterSpawnEntry& EnemyEntry, int32 SpawnSequenceIndex);
	int32 SelectEnemyEntryIndex(int32 RemainingBudget, const TArray<int32>& SpawnCountsByEntry) const;
	bool ResolveEnemySpawnTransform(const FRiftEnemyEncounterSpawnEntry& EnemyEntry, int32 SpawnSequenceIndex, FTransform& OutSpawnTransform) const;

	bool bHasTriggered = false;
};
