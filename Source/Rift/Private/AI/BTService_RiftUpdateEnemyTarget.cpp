#include "AI/BTService_RiftUpdateEnemyTarget.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/EnemyCharacter.h"
#include "Character/PlayerCharacter.h"
#include "Engine/World.h"
#include "EngineUtils.h"

UBTService_RiftUpdateEnemyTarget::UBTService_RiftUpdateEnemyTarget()
{
	NodeName = TEXT("Rift Update Enemy Target");
	Interval = 0.2f;
	RandomDeviation = 0.05f;
	TargetActorKey.SelectedKeyName = TEXT("TargetActor");
	DistanceToTargetKey.SelectedKeyName = TEXT("DistanceToTarget");
}

void UBTService_RiftUpdateEnemyTarget::OnSearchStart(FBehaviorTreeSearchData& SearchData)
{
	Super::OnSearchStart(SearchData);
	UpdateEnemyTarget(SearchData.OwnerComp);
}

void UBTService_RiftUpdateEnemyTarget::TickNode(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory,
	float DeltaSeconds
)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
	UpdateEnemyTarget(OwnerComp);
}

void UBTService_RiftUpdateEnemyTarget::UpdateEnemyTarget(UBehaviorTreeComponent& OwnerComp)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	AEnemyCharacter* EnemyCharacter = AIController ? Cast<AEnemyCharacter>(AIController->GetPawn()) : nullptr;
	UWorld* World = EnemyCharacter ? EnemyCharacter->GetWorld() : nullptr;
	UBlackboardComponent* BlackboardComponent = OwnerComp.GetBlackboardComponent();
	if (!AIController || !EnemyCharacter || !World || !BlackboardComponent) return;

	if (EnemyCharacter->IsDeadForAI() || EnemyCharacter->IsStaggeredForAI())
	{
		BlackboardComponent->ClearValue(TargetActorKey.SelectedKeyName);
		BlackboardComponent->SetValueAsFloat(DistanceToTargetKey.SelectedKeyName, 0.0f);
		AIController->StopMovement();
		return;
	}

	APlayerCharacter* ClosestPlayer = nullptr;
	float ClosestDistanceSq = TNumericLimits<float>::Max();
	const FVector EnemyLocation = EnemyCharacter->GetActorLocation();

	for (TActorIterator<APlayerCharacter> It(World); It; ++It)
	{
		APlayerCharacter* PlayerCharacter = *It;
		if (!PlayerCharacter) continue;
		if (PlayerCharacter->IsDead()) continue;

		FVector ToPlayer = PlayerCharacter->GetActorLocation() - EnemyLocation;
		ToPlayer.Z = 0.0f;
		const float DistanceSq = ToPlayer.SizeSquared();
		if (DistanceSq < ClosestDistanceSq)
		{
			ClosestDistanceSq = DistanceSq;
			ClosestPlayer = PlayerCharacter;
		}
	}

	if (!ClosestPlayer)
	{
		BlackboardComponent->ClearValue(TargetActorKey.SelectedKeyName);
		BlackboardComponent->SetValueAsFloat(DistanceToTargetKey.SelectedKeyName, 0.0f);
		AIController->StopMovement();
		return;
	}

	BlackboardComponent->SetValueAsObject(TargetActorKey.SelectedKeyName, ClosestPlayer);
	BlackboardComponent->SetValueAsFloat(
		DistanceToTargetKey.SelectedKeyName,
		FMath::Sqrt(ClosestDistanceSq)
	);
}
