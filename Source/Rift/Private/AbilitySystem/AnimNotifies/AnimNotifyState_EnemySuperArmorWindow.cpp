#include "AbilitySystem/AnimNotifies/AnimNotifyState_EnemySuperArmorWindow.h"

#include "Character/EnemyCharacter.h"
#include "Components/SkeletalMeshComponent.h"

void UAnimNotifyState_EnemySuperArmorWindow::NotifyBegin(
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
		EnemyCharacter->SetEnemySuperArmorState(true);
	}
}

void UAnimNotifyState_EnemySuperArmorWindow::NotifyEnd(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference
)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (!MeshComp) return;

	if (AEnemyCharacter* EnemyCharacter = Cast<AEnemyCharacter>(MeshComp->GetOwner()))
	{
		EnemyCharacter->SetEnemySuperArmorState(false);
	}
}
