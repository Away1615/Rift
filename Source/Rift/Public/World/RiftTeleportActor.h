#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RiftTeleportActor.generated.h"

class AEnemyCharacter;
class APlayerCharacter;
class UBoxComponent;
class UEnemyCharacterConfig;
class UWidgetComponent;

USTRUCT(BlueprintType)
struct FRiftTeleportEnemySpawnEntry
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
	int32 MaxBudgetSpawnCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Enemy", meta=(ClampMin="0.0"))
	float SpawnRadius = 600.0f;
};

USTRUCT(BlueprintType)
struct FRiftTeleportEnemyWave
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Wave", meta=(ClampMin="0.0", ClampMax="1.0"))
	float TriggerProgress = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Wave", meta=(ClampMin="0"))
	int32 SpawnBudget = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Wave")
	TArray<FRiftTeleportEnemySpawnEntry> EnemyPool;
};

UCLASS(Blueprintable)
class RIFT_API ARiftTeleportActor : public AActor
{
	GENERATED_BODY()

public:
	ARiftTeleportActor();

	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable, Category="Rift|Teleport")
	void StartActivation();

	UFUNCTION(BlueprintCallable, Category="Rift|Teleport")
	void TryTravelToNextLevel(APlayerCharacter* RequestingPlayer);

	UFUNCTION(BlueprintPure, Category="Rift|Teleport")
	float GetActivationProgress() const { return ActivationProgress; }

	UFUNCTION(BlueprintPure, Category="Rift|Teleport")
	bool IsDiscovered() const { return bDiscovered; }

	UFUNCTION(BlueprintPure, Category="Rift|Teleport")
	bool IsActivating() const { return bActivating; }

	UFUNCTION(BlueprintPure, Category="Rift|Teleport")
	bool IsActivated() const { return bActivated; }

	UFUNCTION(BlueprintPure, Category="Rift|Teleport")
	UWidgetComponent* GetProgressWidgetComponent() const { return ProgressWidgetComponent; }

	UFUNCTION(BlueprintImplementableEvent, Category="Rift|Teleport")
	void OnTeleportDiscovered();

	UFUNCTION(BlueprintImplementableEvent, Category="Rift|Teleport")
	void OnActivationStarted();

	UFUNCTION(BlueprintImplementableEvent, Category="Rift|Teleport")
	void OnActivationProgressChanged(float Progress);

	UFUNCTION(BlueprintImplementableEvent, Category="Rift|Teleport")
	void OnEnemyWaveStarted(int32 WaveIndex);

	UFUNCTION(BlueprintImplementableEvent, Category="Rift|Teleport")
	void OnTeleportActivated();

	UFUNCTION(BlueprintImplementableEvent, Category="Rift|Teleport")
	void OnTeleportTravelStarted();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Rift|Teleport")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Rift|Teleport")
	TObjectPtr<UBoxComponent> TriggerBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Rift|Teleport")
	TObjectPtr<UWidgetComponent> ProgressWidgetComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Rift|Teleport", meta=(ClampMin="0.0"))
	float ActivationDuration = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Rift|Teleport", meta=(ClampMin="0.01"))
	float ActivationTickInterval = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Rift|Teleport")
	FString NextLevelTravelPath = TEXT("/Game/0_/Map/Level_2");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Rift|Teleport")
	bool bAppendListenOption = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Rift|Teleport|Waves")
	TArray<FRiftTeleportEnemyWave> EnemyWaves;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Rift|Teleport|Waves")
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

	UFUNCTION()
	void OnRep_Discovered();

	UFUNCTION()
	void OnRep_Activating();

	UFUNCTION()
	void OnRep_ActivationProgress();

	UFUNCTION()
	void OnRep_Activated();

	void SetDiscovered();
	void TickActivation();
	void CompleteActivation();
	void TryTravelOverlappingPlayer();
	void TryStartEnemyWaves();
	void SpawnEnemyWave(int32 WaveIndex);
	void SpawnBudgetEnemyWave(const FRiftTeleportEnemyWave& Wave);
	bool SpawnEnemyFromEntry(const FRiftTeleportEnemySpawnEntry& EnemyEntry, int32 SpawnSequenceIndex);
	const FRiftTeleportEnemySpawnEntry* SelectBudgetEnemyEntry(
		const FRiftTeleportEnemyWave& Wave,
		int32 RemainingBudget,
		const TArray<int32>& SpawnCountsByEntry
	) const;
	bool ResolveEnemySpawnTransform(const FRiftTeleportEnemySpawnEntry& EnemyEntry, int32 SpawnSequenceIndex, FTransform& OutSpawnTransform) const;
	void ServerTravelToNextLevel();

	UPROPERTY(ReplicatedUsing=OnRep_Discovered)
	bool bDiscovered = false;

	UPROPERTY(ReplicatedUsing=OnRep_Activating)
	bool bActivating = false;

	UPROPERTY(ReplicatedUsing=OnRep_Activated)
	bool bActivated = false;

	UPROPERTY(ReplicatedUsing=OnRep_ActivationProgress)
	float ActivationProgress = 0.0f;

	FTimerHandle ActivationTimerHandle;
	TSet<int32> SpawnedWaveIndices;
	bool bTravelStarted = false;
};
