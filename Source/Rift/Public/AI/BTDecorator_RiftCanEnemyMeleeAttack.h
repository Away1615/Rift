#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "BTDecorator_RiftCanEnemyMeleeAttack.generated.h"

UCLASS()
class RIFT_API UBTDecorator_RiftCanEnemyMeleeAttack : public UBTDecorator
{
	GENERATED_BODY()

public:
	UBTDecorator_RiftCanEnemyMeleeAttack();

protected:
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;

	UPROPERTY(EditAnywhere, Category="Blackboard")
	FBlackboardKeySelector TargetActorKey;
};
