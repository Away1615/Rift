#include "AbilitySystem/AnimNotifies/AnimNotifyState_EnemyMeleeHitWindow.h"

#include "Character/EnemyCharacter.h"
#include "Components/SkeletalMeshComponent.h"

void UAnimNotifyState_EnemyMeleeHitWindow::NotifyBegin(
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
		EnemyCharacter->BeginAttackHitWindow();
	}
}

void UAnimNotifyState_EnemyMeleeHitWindow::NotifyTick(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	float FrameDeltaTime,
	const FAnimNotifyEventReference& EventReference
)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);

	if (!MeshComp) return;

	if (AEnemyCharacter* EnemyCharacter = Cast<AEnemyCharacter>(MeshComp->GetOwner()))
	{
		EnemyCharacter->TickAttackHitWindow();
	}
}

void UAnimNotifyState_EnemyMeleeHitWindow::NotifyEnd(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference
)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (!MeshComp) return;

	if (AEnemyCharacter* EnemyCharacter = Cast<AEnemyCharacter>(MeshComp->GetOwner()))
	{
		EnemyCharacter->EndAttackHitWindow();
	}
}
