#include "AbilitySystem/AnimNotifies/AnimNotifyState_ComboChainWindow.h"

#include "AbilitySystem/Abilities/GA_TwinSwordComboAttack.h"
#include "AbilitySystem/Abilities/GA_TwinSwordRapidSlash.h"
#include "Components/SkeletalMeshComponent.h"

void UAnimNotifyState_ComboChainWindow::NotifyBegin(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	float TotalDuration,
	const FAnimNotifyEventReference& EventReference
)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (!MeshComp) return;

	if (UGA_TwinSwordComboAttack* TwinSwordComboAttack =
		UGA_TwinSwordComboAttack::FindActiveTwinSwordComboInstance(MeshComp->GetOwner()))
	{
		TwinSwordComboAttack->OpenComboChainWindow();
		return;
	}

	if (UGA_TwinSwordRapidSlash* RapidSlash =
		UGA_TwinSwordRapidSlash::FindActiveRapidSlashInstance(MeshComp->GetOwner()))
	{
		RapidSlash->OpenComboChainWindow();
	}
}

void UAnimNotifyState_ComboChainWindow::NotifyEnd(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference
)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (!MeshComp) return;

	if (UGA_TwinSwordComboAttack* TwinSwordComboAttack =
		UGA_TwinSwordComboAttack::FindActiveTwinSwordComboInstance(MeshComp->GetOwner()))
	{
		TwinSwordComboAttack->CloseComboChainWindow();
		return;
	}

	if (UGA_TwinSwordRapidSlash* RapidSlash =
		UGA_TwinSwordRapidSlash::FindActiveRapidSlashInstance(MeshComp->GetOwner()))
	{
		RapidSlash->CloseComboChainWindow();
	}
}
