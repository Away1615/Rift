#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "BTTask_RiftTryEnemyShieldBlock.generated.h"

UCLASS()
class RIFT_API UBTTask_RiftTryEnemyShieldBlock : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_RiftTryEnemyShieldBlock();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	UPROPERTY(EditAnywhere, Category="Blackboard")
	FBlackboardKeySelector TargetActorKey;
};
