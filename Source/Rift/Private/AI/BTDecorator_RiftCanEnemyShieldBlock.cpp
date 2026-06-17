#include "AI/BTDecorator_RiftCanEnemyShieldBlock.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/EnemyCharacter.h"

UBTDecorator_RiftCanEnemyShieldBlock::UBTDecorator_RiftCanEnemyShieldBlock()
{
	NodeName = TEXT("Rift Can Enemy Shield Block");
	TargetActorKey.SelectedKeyName = TEXT("TargetActor");
}

bool UBTDecorator_RiftCanEnemyShieldBlock::CalculateRawConditionValue(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory
) const
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	AEnemyCharacter* EnemyCharacter = AIController ? Cast<AEnemyCharacter>(AIController->GetPawn()) : nullptr;
	UBlackboardComponent* BlackboardComponent = OwnerComp.GetBlackboardComponent();
	if (!EnemyCharacter || !BlackboardComponent) return false;

	AActor* TargetActor = Cast<AActor>(BlackboardComponent->GetValueAsObject(TargetActorKey.SelectedKeyName));
	return EnemyCharacter->CanStartShieldBlock(TargetActor);
}
