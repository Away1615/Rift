#include "Animation/Notify/AnimNotifyState_WeaponTraceWindow.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameplayTags/RiftGameplayTags.h"

void UAnimNotifyState_WeaponTraceWindow::NotifyBegin(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	float TotalDuration,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	SendGameplayEvent(MeshComp, FRiftGameplayTags::Get().Event_Ability_TwinSword_Combo_WeaponTraceBegin);
}

void UAnimNotifyState_WeaponTraceWindow::NotifyTick(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	float FrameDeltaTime,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);
	SendGameplayEvent(MeshComp, FRiftGameplayTags::Get().Event_Ability_TwinSword_Combo_WeaponTraceTick);
}

void UAnimNotifyState_WeaponTraceWindow::NotifyEnd(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);
	SendGameplayEvent(MeshComp, FRiftGameplayTags::Get().Event_Ability_TwinSword_Combo_WeaponTraceEnd);
}

void UAnimNotifyState_WeaponTraceWindow::SendGameplayEvent(
	USkeletalMeshComponent* MeshComp,
	const FGameplayTag& EventTag)
{
	if (!MeshComp || !EventTag.IsValid()) return;

	AActor* Owner = MeshComp->GetOwner();
	if (!Owner) return;

	FGameplayEventData EventData;
	EventData.EventTag = EventTag;
	EventData.Instigator = Owner;
	EventData.Target = Owner;

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Owner, EventTag, EventData);
}
