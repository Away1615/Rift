#include "AI/RiftEnemyAIController.h"

#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Character/EnemyCharacter.h"
#include "Data/Enemy/EnemyCharacterConfig.h"

ARiftEnemyAIController::ARiftEnemyAIController()
{
	bAttachToPawn = true;
}

void ARiftEnemyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	EnemyCharacter = Cast<AEnemyCharacter>(InPawn);
}

void ARiftEnemyAIController::OnUnPossess()
{
	StopMovement();
	if (UBehaviorTreeComponent* BehaviorTreeComponent = Cast<UBehaviorTreeComponent>(BrainComponent))
	{
		BehaviorTreeComponent->StopTree();
	}
	EnemyCharacter = nullptr;

	Super::OnUnPossess();
}

void ARiftEnemyAIController::StartEnemyBehavior(AEnemyCharacter* InEnemyCharacter)
{
	if (!HasAuthority()) return;

	EnemyCharacter = InEnemyCharacter;
	if (!EnemyCharacter) return;

	const UEnemyCharacterConfig* CharacterConfig = EnemyCharacter->GetEnemyCharacterConfig();
	UBehaviorTree* BehaviorTree = CharacterConfig ? CharacterConfig->BehaviorTree : nullptr;
	if (!BehaviorTree)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s has no BehaviorTree configured."), *EnemyCharacter->GetName());
		return;
	}

	RunBehaviorTree(BehaviorTree);
}
