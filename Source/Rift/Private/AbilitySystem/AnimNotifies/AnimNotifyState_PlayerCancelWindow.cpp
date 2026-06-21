#include "AbilitySystem/AnimNotifies/AnimNotifyState_PlayerCancelWindow.h"

#include "Character/PlayerCharacter.h"
#include "Components/SkeletalMeshComponent.h"

void UAnimNotifyState_PlayerCancelWindow::NotifyBegin(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	float TotalDuration,
	const FAnimNotifyEventReference& EventReference
)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	APlayerCharacter* PlayerCharacter = MeshComp ? Cast<APlayerCharacter>(MeshComp->GetOwner()) : nullptr;
	if (!PlayerCharacter) return;

	PlayerCharacter->SetActionCancelableState(true);
}

void UAnimNotifyState_PlayerCancelWindow::NotifyEnd(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference
)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	APlayerCharacter* PlayerCharacter = MeshComp ? Cast<APlayerCharacter>(MeshComp->GetOwner()) : nullptr;
	if (!PlayerCharacter) return;

	PlayerCharacter->SetActionCancelableState(false);
}
