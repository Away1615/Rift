#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "RiftEnemyAIController.generated.h"

class AEnemyCharacter;
class APlayerCharacter;

UCLASS()
class RIFT_API ARiftEnemyAIController : public AAIController
{
	GENERATED_BODY()

public:
	ARiftEnemyAIController();
	void StartEnemyBehavior(AEnemyCharacter* InEnemyCharacter);

protected:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

private:
	UPROPERTY(Transient)
	TObjectPtr<AEnemyCharacter> EnemyCharacter;
};
