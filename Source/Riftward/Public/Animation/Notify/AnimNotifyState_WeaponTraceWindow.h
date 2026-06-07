#pragma once

#include "CoreMinimal.h"
#include "Animation/Notify/AnimNotifyState_SendGameplayEventWindow.h"
#include "AnimNotifyState_WeaponTraceWindow.generated.h"

UCLASS(DisplayName="GAS | Weapon Trace Window")
class RIFTWARD_API UAnimNotifyState_WeaponTraceWindow : public UAnimNotifyState_SendGameplayEventWindow
{
	GENERATED_BODY()

public:
	UAnimNotifyState_WeaponTraceWindow();

	// -1 = TraceBoth, 0 = RightSword, 1 = LeftSword
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon Trace", meta=(ClampMin="-1", ClampMax="1"))
	int32 WeaponIndex = INDEX_NONE;

	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
