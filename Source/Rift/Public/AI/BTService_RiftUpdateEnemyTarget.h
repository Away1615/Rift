#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "BTService_RiftUpdateEnemyTarget.generated.h"

UCLASS()
class RIFT_API UBTService_RiftUpdateEnemyTarget : public UBTService
{
	GENERATED_BODY()

public:
	UBTService_RiftUpdateEnemyTarget();

protected:
	virtual void OnSearchStart(FBehaviorTreeSearchData& SearchData) override;
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	UPROPERTY(EditAnywhere, Category="Blackboard")
	FBlackboardKeySelector TargetActorKey;

	UPROPERTY(EditAnywhere, Category="Blackboard")
	FBlackboardKeySelector DistanceToTargetKey;

private:
	void UpdateEnemyTarget(UBehaviorTreeComponent& OwnerComp);
};
