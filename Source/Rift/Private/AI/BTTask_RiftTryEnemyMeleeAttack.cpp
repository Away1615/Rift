#include "AI/BTTask_RiftTryEnemyMeleeAttack.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/EnemyCharacter.h"

UBTTask_RiftTryEnemyMeleeAttack::UBTTask_RiftTryEnemyMeleeAttack()
{
	NodeName = TEXT("Rift Try Enemy Melee Attack");
	TargetActorKey.SelectedKeyName = TEXT("TargetActor");
}

EBTNodeResult::Type UBTTask_RiftTryEnemyMeleeAttack::ExecuteTask(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory
)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	AEnemyCharacter* EnemyCharacter = AIController ? Cast<AEnemyCharacter>(AIController->GetPawn()) : nullptr;
	UBlackboardComponent* BlackboardComponent = OwnerComp.GetBlackboardComponent();
	if (!EnemyCharacter || !BlackboardComponent) return EBTNodeResult::Failed;

	AActor* TargetActor = Cast<AActor>(BlackboardComponent->GetValueAsObject(TargetActorKey.SelectedKeyName));
	return EnemyCharacter->TryStartMeleeAttack(TargetActor)
		? EBTNodeResult::Succeeded
		: EBTNodeResult::Failed;
}
