#include "AI/BTTask_RiftTryEnemyShieldBlock.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/EnemyCharacter.h"

UBTTask_RiftTryEnemyShieldBlock::UBTTask_RiftTryEnemyShieldBlock()
{
	NodeName = TEXT("Rift Try Enemy Shield Block");
	TargetActorKey.SelectedKeyName = TEXT("TargetActor");
}

EBTNodeResult::Type UBTTask_RiftTryEnemyShieldBlock::ExecuteTask(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory
)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	AEnemyCharacter* EnemyCharacter = AIController ? Cast<AEnemyCharacter>(AIController->GetPawn()) : nullptr;
	UBlackboardComponent* BlackboardComponent = OwnerComp.GetBlackboardComponent();
	if (!EnemyCharacter || !BlackboardComponent) return EBTNodeResult::Failed;

	AActor* TargetActor = Cast<AActor>(BlackboardComponent->GetValueAsObject(TargetActorKey.SelectedKeyName));
	return EnemyCharacter->TryStartShieldBlock(TargetActor)
		? EBTNodeResult::Succeeded
		: EBTNodeResult::Failed;
}
