#include "Animation/Notify/AnimNotifyState_WeaponTraceWindow.h"
#include "GameplayTags/RiftGameplayTags.h"
#include "Tools/GameTools.h"

UAnimNotifyState_WeaponTraceWindow::UAnimNotifyState_WeaponTraceWindow()
{
	const FRiftGameplayTags& Tags = FRiftGameplayTags::Get();
	BeginEventTag = Tags.Event_TwinSword_Combo_WeaponTrace_Begin;
	TickEventTag = Tags.Event_TwinSword_Combo_WeaponTrace_Tick;
	EndEventTag = Tags.Event_TwinSword_Combo_WeaponTrace_End;
}

void UAnimNotifyState_WeaponTraceWindow::NotifyBegin(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	float TotalDuration,
	const FAnimNotifyEventReference& EventReference)
{
	UAnimNotifyState::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	GameTools::SendGameplayEvent(MeshComp, BeginEventTag, static_cast<float>(WeaponIndex));
}

void UAnimNotifyState_WeaponTraceWindow::NotifyTick(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	float FrameDeltaTime,
	const FAnimNotifyEventReference& EventReference)
{
	UAnimNotifyState::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);
	GameTools::SendGameplayEvent(MeshComp, TickEventTag, static_cast<float>(WeaponIndex));
}

void UAnimNotifyState_WeaponTraceWindow::NotifyEnd(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	UAnimNotifyState::NotifyEnd(MeshComp, Animation, EventReference);
	GameTools::SendGameplayEvent(MeshComp, EndEventTag, static_cast<float>(WeaponIndex));
}
