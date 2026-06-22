#include "World/RiftEnemyEncounterTrigger.h"

#include "Character/EnemyCharacter.h"
#include "Character/PlayerCharacter.h"
#include "Components/BoxComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"

ARiftEnemyEncounterTrigger::ARiftEnemyEncounterTrigger()
{
	bReplicates = true;
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	TriggerBox->SetupAttachment(SceneRoot);
	TriggerBox->SetBoxExtent(FVector(200.0f, 200.0f, 120.0f));
	TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

void ARiftEnemyEncounterTrigger::BeginPlay()
{
	Super::BeginPlay();

	if (TriggerBox)
	{
		TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &ARiftEnemyEncounterTrigger::HandleTriggerBeginOverlap);
	}
}

void ARiftEnemyEncounterTrigger::HandleTriggerBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	const int32 OtherBodyIndex,
	const bool bFromSweep,
	const FHitResult& SweepResult
)
{
	static_cast<void>(OverlappedComponent);
	static_cast<void>(OtherComp);
	static_cast<void>(OtherBodyIndex);
	static_cast<void>(bFromSweep);
	static_cast<void>(SweepResult);

	if (!Cast<APlayerCharacter>(OtherActor))
	{
		return;
	}

	TriggerEncounter();
}

void ARiftEnemyEncounterTrigger::TriggerEncounter()
{
	if (!HasAuthority())
	{
		return;
	}

	if (bTriggerOnce && bHasTriggered)
	{
		return;
	}

	bHasTriggered = true;
	OnEncounterTriggered();
	SpawnEncounterEnemies();
}

void ARiftEnemyEncounterTrigger::SpawnEncounterEnemies()
{
	int32 RemainingBudget = SpawnBudget;
	TArray<int32> SpawnCountsByEntry;
	SpawnCountsByEntry.SetNumZeroed(EnemyPool.Num());

	int32 SpawnSequenceIndex = 0;
	while (RemainingBudget > 0)
	{
		const int32 EntryIndex = SelectEnemyEntryIndex(RemainingBudget, SpawnCountsByEntry);
		if (!EnemyPool.IsValidIndex(EntryIndex))
		{
			break;
		}

		const FRiftEnemyEncounterSpawnEntry& EnemyEntry = EnemyPool[EntryIndex];
		if (SpawnEnemyFromEntry(EnemyEntry, SpawnSequenceIndex))
		{
			RemainingBudget -= FMath::Max(1, EnemyEntry.SpawnCost);
			++SpawnCountsByEntry[EntryIndex];
			++SpawnSequenceIndex;
		}
		else
		{
			break;
		}
	}
}

bool ARiftEnemyEncounterTrigger::SpawnEnemyFromEntry(
	const FRiftEnemyEncounterSpawnEntry& EnemyEntry,
	const int32 SpawnSequenceIndex
)
{
	if (!EnemyEntry.EnemyClass || !EnemyEntry.EnemyConfig)
	{
		return false;
	}

	FTransform SpawnTransform;
	if (!ResolveEnemySpawnTransform(EnemyEntry, SpawnSequenceIndex, SpawnTransform))
	{
		return false;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	AEnemyCharacter* EnemyCharacter = World->SpawnActorDeferred<AEnemyCharacter>(
		EnemyEntry.EnemyClass,
		SpawnTransform,
		nullptr,
		nullptr,
		ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn
	);
	if (!EnemyCharacter)
	{
		return false;
	}

	EnemyCharacter->SetEnemyCharacterConfig(EnemyEntry.EnemyConfig);
	UGameplayStatics::FinishSpawningActor(EnemyCharacter, SpawnTransform);
	OnEncounterEnemySpawned(EnemyCharacter);
	return true;
}

int32 ARiftEnemyEncounterTrigger::SelectEnemyEntryIndex(
	const int32 RemainingBudget,
	const TArray<int32>& SpawnCountsByEntry
) const
{
	float TotalWeight = 0.0f;
	for (int32 EntryIndex = 0; EntryIndex < EnemyPool.Num(); ++EntryIndex)
	{
		const FRiftEnemyEncounterSpawnEntry& EnemyEntry = EnemyPool[EntryIndex];
		if (!EnemyEntry.EnemyClass || !EnemyEntry.EnemyConfig)
		{
			continue;
		}

		if (EnemyEntry.SpawnCost > RemainingBudget || EnemyEntry.SpawnWeight <= 0.0f)
		{
			continue;
		}

		if (EnemyEntry.MaxSpawnCount > 0 && SpawnCountsByEntry[EntryIndex] >= EnemyEntry.MaxSpawnCount)
		{
			continue;
		}

		TotalWeight += EnemyEntry.SpawnWeight;
	}

	if (TotalWeight <= 0.0f)
	{
		return INDEX_NONE;
	}

	float Roll = FMath::FRandRange(0.0f, TotalWeight);
	for (int32 EntryIndex = 0; EntryIndex < EnemyPool.Num(); ++EntryIndex)
	{
		const FRiftEnemyEncounterSpawnEntry& EnemyEntry = EnemyPool[EntryIndex];
		if (!EnemyEntry.EnemyClass || !EnemyEntry.EnemyConfig)
		{
			continue;
		}

		if (EnemyEntry.SpawnCost > RemainingBudget || EnemyEntry.SpawnWeight <= 0.0f)
		{
			continue;
		}

		if (EnemyEntry.MaxSpawnCount > 0 && SpawnCountsByEntry[EntryIndex] >= EnemyEntry.MaxSpawnCount)
		{
			continue;
		}

		Roll -= EnemyEntry.SpawnWeight;
		if (Roll <= 0.0f)
		{
			return EntryIndex;
		}
	}

	return INDEX_NONE;
}

bool ARiftEnemyEncounterTrigger::ResolveEnemySpawnTransform(
	const FRiftEnemyEncounterSpawnEntry& EnemyEntry,
	const int32 SpawnSequenceIndex,
	FTransform& OutSpawnTransform
) const
{
	if (SpawnPoints.IsEmpty())
	{
		return false;
	}

	UWorld* World = GetWorld();
	UNavigationSystemV1* NavigationSystem = World ? FNavigationSystem::GetCurrent<UNavigationSystemV1>(World) : nullptr;
	if (!NavigationSystem)
	{
		return false;
	}

	const int32 SpawnPointIndex = SpawnSequenceIndex % SpawnPoints.Num();
	AActor* SpawnPoint = SpawnPoints.IsValidIndex(SpawnPointIndex) ? SpawnPoints[SpawnPointIndex] : nullptr;
	if (!SpawnPoint)
	{
		return false;
	}

	FNavLocation NavLocation;
	if (!NavigationSystem->GetRandomReachablePointInRadius(
		SpawnPoint->GetActorLocation(),
		FMath::Max(1.0f, EnemyEntry.SpawnRadius),
		NavLocation
	))
	{
		return false;
	}

	OutSpawnTransform = SpawnPoint->GetActorTransform();
	OutSpawnTransform.SetLocation(NavLocation.Location);
	return true;
}
