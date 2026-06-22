#include "World/RiftTeleportActor.h"

#include "Character/EnemyCharacter.h"
#include "Character/PlayerCharacter.h"
#include "Components/BoxComponent.h"
#include "Components/WidgetComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "Net/UnrealNetwork.h"

ARiftTeleportActor::ARiftTeleportActor()
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

	ProgressWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("ProgressWidget"));
	ProgressWidgetComponent->SetupAttachment(SceneRoot);
	ProgressWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
	ProgressWidgetComponent->SetDrawAtDesiredSize(true);
	ProgressWidgetComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 180.0f));
}

void ARiftTeleportActor::BeginPlay()
{
	Super::BeginPlay();

	if (TriggerBox)
	{
		TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &ARiftTeleportActor::HandleTriggerBeginOverlap);
	}
}

void ARiftTeleportActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ARiftTeleportActor, bDiscovered);
	DOREPLIFETIME(ARiftTeleportActor, bActivating);
	DOREPLIFETIME(ARiftTeleportActor, bActivated);
	DOREPLIFETIME(ARiftTeleportActor, ActivationProgress);
}

void ARiftTeleportActor::HandleTriggerBeginOverlap(
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

	APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(OtherActor);
	if (!PlayerCharacter)
	{
		return;
	}

	if (bActivated)
	{
		TryTravelToNextLevel(PlayerCharacter);
		return;
	}

	if (!bDiscovered)
	{
		SetDiscovered();
		StartActivation();
	}
}

void ARiftTeleportActor::StartActivation()
{
	if (!HasAuthority() || bActivated || bActivating)
	{
		return;
	}

	SetDiscovered();
	bActivating = true;
	OnActivationStarted();
	ForceNetUpdate();

	if (ActivationDuration <= 0.0f)
	{
		CompleteActivation();
		return;
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			ActivationTimerHandle,
			this,
			&ARiftTeleportActor::TickActivation,
			FMath::Max(0.01f, ActivationTickInterval),
			true
		);
	}
}

void ARiftTeleportActor::TryTravelToNextLevel(APlayerCharacter* RequestingPlayer)
{
	if (!HasAuthority() || !bActivated || bTravelStarted || !RequestingPlayer)
	{
		return;
	}

	ServerTravelToNextLevel();
}

void ARiftTeleportActor::SetDiscovered()
{
	if (!HasAuthority() || bDiscovered)
	{
		return;
	}

	bDiscovered = true;
	OnTeleportDiscovered();
	ForceNetUpdate();
}

void ARiftTeleportActor::TickActivation()
{
	if (!HasAuthority() || !bActivating || bActivated)
	{
		return;
	}

	const float DeltaProgress = FMath::Max(0.01f, ActivationTickInterval) / FMath::Max(0.01f, ActivationDuration);
	ActivationProgress = FMath::Clamp(ActivationProgress + DeltaProgress, 0.0f, 1.0f);
	OnActivationProgressChanged(ActivationProgress);
	TryStartEnemyWaves();
	ForceNetUpdate();

	if (ActivationProgress >= 1.0f)
	{
		CompleteActivation();
	}
}

void ARiftTeleportActor::CompleteActivation()
{
	if (!HasAuthority() || bActivated)
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ActivationTimerHandle);
	}

	ActivationProgress = 1.0f;
	bActivating = false;
	bActivated = true;
	OnActivationProgressChanged(ActivationProgress);
	OnTeleportActivated();
	ForceNetUpdate();
	TryTravelOverlappingPlayer();
}

void ARiftTeleportActor::TryTravelOverlappingPlayer()
{
	if (!HasAuthority() || !bActivated || bTravelStarted || !TriggerBox)
	{
		return;
	}

	TArray<AActor*> OverlappingActors;
	TriggerBox->GetOverlappingActors(OverlappingActors, APlayerCharacter::StaticClass());

	for (AActor* OverlappingActor : OverlappingActors)
	{
		if (APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(OverlappingActor))
		{
			TryTravelToNextLevel(PlayerCharacter);
			return;
		}
	}
}

void ARiftTeleportActor::TryStartEnemyWaves()
{
	for (int32 WaveIndex = 0; WaveIndex < EnemyWaves.Num(); ++WaveIndex)
	{
		if (SpawnedWaveIndices.Contains(WaveIndex))
		{
			continue;
		}

		if (ActivationProgress >= EnemyWaves[WaveIndex].TriggerProgress)
		{
			SpawnEnemyWave(WaveIndex);
		}
	}
}

void ARiftTeleportActor::SpawnEnemyWave(const int32 WaveIndex)
{
	if (!HasAuthority() || !EnemyWaves.IsValidIndex(WaveIndex))
	{
		return;
	}

	SpawnedWaveIndices.Add(WaveIndex);
	OnEnemyWaveStarted(WaveIndex);

	UWorld* World = GetWorld();
	if (!World || SpawnPoints.IsEmpty())
	{
		return;
	}

	const FRiftTeleportEnemyWave& Wave = EnemyWaves[WaveIndex];
	SpawnBudgetEnemyWave(Wave);
}

void ARiftTeleportActor::SpawnBudgetEnemyWave(const FRiftTeleportEnemyWave& Wave)
{
	int32 RemainingBudget = Wave.SpawnBudget;
	TArray<int32> SpawnCountsByEntry;
	SpawnCountsByEntry.SetNumZeroed(Wave.EnemyPool.Num());

	int32 SpawnSequenceIndex = 0;
	while (RemainingBudget > 0)
	{
		const FRiftTeleportEnemySpawnEntry* EnemyEntry = SelectBudgetEnemyEntry(
			Wave,
			RemainingBudget,
			SpawnCountsByEntry
		);
		if (!EnemyEntry)
		{
			break;
		}

		const int32 EntryIndex = static_cast<int32>(EnemyEntry - Wave.EnemyPool.GetData());
		if (SpawnEnemyFromEntry(*EnemyEntry, SpawnSequenceIndex))
		{
			RemainingBudget -= FMath::Max(1, EnemyEntry->SpawnCost);
			++SpawnCountsByEntry[EntryIndex];
			++SpawnSequenceIndex;
		}
		else
		{
			break;
		}
	}
}

bool ARiftTeleportActor::SpawnEnemyFromEntry(
	const FRiftTeleportEnemySpawnEntry& EnemyEntry,
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
	return true;
}

const FRiftTeleportEnemySpawnEntry* ARiftTeleportActor::SelectBudgetEnemyEntry(
	const FRiftTeleportEnemyWave& Wave,
	const int32 RemainingBudget,
	const TArray<int32>& SpawnCountsByEntry
) const
{
	float TotalWeight = 0.0f;
	for (int32 EntryIndex = 0; EntryIndex < Wave.EnemyPool.Num(); ++EntryIndex)
	{
		const FRiftTeleportEnemySpawnEntry& EnemyEntry = Wave.EnemyPool[EntryIndex];
		if (!EnemyEntry.EnemyClass || !EnemyEntry.EnemyConfig)
		{
			continue;
		}

		if (EnemyEntry.SpawnCost > RemainingBudget || EnemyEntry.SpawnWeight <= 0.0f)
		{
			continue;
		}

		if (EnemyEntry.MaxBudgetSpawnCount > 0 && SpawnCountsByEntry[EntryIndex] >= EnemyEntry.MaxBudgetSpawnCount)
		{
			continue;
		}

		TotalWeight += EnemyEntry.SpawnWeight;
	}

	if (TotalWeight <= 0.0f)
	{
		return nullptr;
	}

	float Roll = FMath::FRandRange(0.0f, TotalWeight);
	for (int32 EntryIndex = 0; EntryIndex < Wave.EnemyPool.Num(); ++EntryIndex)
	{
		const FRiftTeleportEnemySpawnEntry& EnemyEntry = Wave.EnemyPool[EntryIndex];
		if (!EnemyEntry.EnemyClass || !EnemyEntry.EnemyConfig)
		{
			continue;
		}

		if (EnemyEntry.SpawnCost > RemainingBudget || EnemyEntry.SpawnWeight <= 0.0f)
		{
			continue;
		}

		if (EnemyEntry.MaxBudgetSpawnCount > 0 && SpawnCountsByEntry[EntryIndex] >= EnemyEntry.MaxBudgetSpawnCount)
		{
			continue;
		}

		Roll -= EnemyEntry.SpawnWeight;
		if (Roll <= 0.0f)
		{
			return &EnemyEntry;
		}
	}

	return nullptr;
}

bool ARiftTeleportActor::ResolveEnemySpawnTransform(
	const FRiftTeleportEnemySpawnEntry& EnemyEntry,
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

void ARiftTeleportActor::ServerTravelToNextLevel()
{
	if (!HasAuthority() || bTravelStarted || NextLevelTravelPath.IsEmpty())
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	bTravelStarted = true;
	OnTeleportTravelStarted();

	FString TravelPath = NextLevelTravelPath;
	if (bAppendListenOption && !TravelPath.Contains(TEXT("?")))
	{
		TravelPath += TEXT("?listen");
	}

	World->ServerTravel(TravelPath);
}

void ARiftTeleportActor::OnRep_Discovered()
{
	if (bDiscovered)
	{
		OnTeleportDiscovered();
	}
}

void ARiftTeleportActor::OnRep_Activating()
{
	if (bActivating)
	{
		OnActivationStarted();
	}
}

void ARiftTeleportActor::OnRep_ActivationProgress()
{
	OnActivationProgressChanged(ActivationProgress);
}

void ARiftTeleportActor::OnRep_Activated()
{
	if (bActivated)
	{
		OnTeleportActivated();
	}
}
