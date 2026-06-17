#include "AbilitySystem/AnimNotifies/AnimNotifyState_EnemyBlockingWindow.h"

#include "Character/EnemyCharacter.h"
#include "Components/SkeletalMeshComponent.h"

void UAnimNotifyState_EnemyBlockingWindow::NotifyBegin(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	float TotalDuration,
	const FAnimNotifyEventReference& EventReference
)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (!MeshComp) return;

	if (AEnemyCharacter* EnemyCharacter = Cast<AEnemyCharacter>(MeshComp->GetOwner()))
	{
		EnemyCharacter->SetBlockingState(true);
	}
}

void UAnimNotifyState_EnemyBlockingWindow::NotifyEnd(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference
)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (!MeshComp) return;

	if (AEnemyCharacter* EnemyCharacter = Cast<AEnemyCharacter>(MeshComp->GetOwner()))
	{
		EnemyCharacter->SetBlockingState(false);
	}
}
