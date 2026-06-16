#include "AI/BTDecorator_RiftCanEnemyMeleeAttack.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/EnemyCharacter.h"

UBTDecorator_RiftCanEnemyMeleeAttack::UBTDecorator_RiftCanEnemyMeleeAttack()
{
	NodeName = TEXT("Rift Can Enemy Melee Attack");
	TargetActorKey.SelectedKeyName = TEXT("TargetActor");
}

bool UBTDecorator_RiftCanEnemyMeleeAttack::CalculateRawConditionValue(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory
) const
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	AEnemyCharacter* EnemyCharacter = AIController ? Cast<AEnemyCharacter>(AIController->GetPawn()) : nullptr;
	UBlackboardComponent* BlackboardComponent = OwnerComp.GetBlackboardComponent();
	if (!EnemyCharacter || !BlackboardComponent) return false;

	AActor* TargetActor = Cast<AActor>(BlackboardComponent->GetValueAsObject(TargetActorKey.SelectedKeyName));
	return EnemyCharacter->CanStartMeleeAttack(TargetActor);
}
