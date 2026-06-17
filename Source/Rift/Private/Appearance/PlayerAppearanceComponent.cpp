#include "Appearance/PlayerAppearanceComponent.h"

#include "Components/SkeletalMeshComponent.h"
#include "Engine/AssetManager.h"
#include "Engine/DataTable.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StreamableManager.h"

UPlayerAppearanceComponent::UPlayerAppearanceComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPlayerAppearanceComponent::SetMasterMeshComponent(USkeletalMeshComponent* InMasterMeshComponent)
{
	MasterMeshComponent = InMasterMeshComponent;
	RefreshLeaderPose();
}

void UPlayerAppearanceComponent::RegisterAppearanceMeshComponent(
	const ERiftPlayerAppearanceSlot Slot,
	USkeletalMeshComponent* MeshComponent)
{
	if (!MeshComponent)
	{
		UnregisterAppearanceMeshComponent(Slot);
		return;
	}

	MeshComponentsBySlot.Add(Slot, MeshComponent);
	MeshComponent->SetLeaderPoseComponent(MasterMeshComponent);
}

void UPlayerAppearanceComponent::UnregisterAppearanceMeshComponent(const ERiftPlayerAppearanceSlot Slot)
{
	CancelActiveLoadForSlot(Slot);
	PendingPartIdsBySlot.Remove(Slot);
	MeshComponentsBySlot.Remove(Slot);
}

bool UPlayerAppearanceComponent::ApplyAppearanceSelection(const FRiftPlayerAppearanceSelection& Selection)
{
	bool bAppliedAny = false;

	if (ApplyAppearancePart(ERiftPlayerAppearanceSlot::Hair, Selection.HairId))
	{
		bAppliedAny = true;
	}

	if (ApplyAppearancePart(ERiftPlayerAppearanceSlot::ArmUpperLeft, Selection.ArmUpperLeftId))
	{
		bAppliedAny = true;
	}

	if (ApplyAppearancePart(ERiftPlayerAppearanceSlot::ArmUpperRight, Selection.ArmUpperRightId))
	{
		bAppliedAny = true;
	}

	return bAppliedAny;
}

bool UPlayerAppearanceComponent::ApplyAppearancePart(
	const ERiftPlayerAppearanceSlot Slot,
	const FName PartId)
{
	USkeletalMeshComponent* TargetComponent = GetComponentForSlot(Slot);
	if (!TargetComponent)
	{
		return false;
	}

	if (PartId == NAME_None)
	{
		CancelActiveLoadForSlot(Slot);
		PendingPartIdsBySlot.Add(Slot, NAME_None);
		TargetComponent->SetSkeletalMesh(nullptr);
		return true;
	}

	if (!AppearanceTable)
	{
		return false;
	}

	const FRiftPlayerAppearancePartRow* Row =
		AppearanceTable->FindRow<FRiftPlayerAppearancePartRow>(PartId, TEXT(""), false);
	if (!Row || Row->Slot != Slot)
	{
		return false;
	}

	const FSoftObjectPath MeshPath = Row->SkeletalMesh.ToSoftObjectPath();
	if (!MeshPath.IsValid())
	{
		return false;
	}

	CancelActiveLoadForSlot(Slot);
	PendingPartIdsBySlot.Add(Slot, PartId);

	FStreamableManager& StreamableManager = UAssetManager::GetStreamableManager();
	TSharedPtr<FStreamableHandle> LoadHandle = StreamableManager.RequestAsyncLoad(
		MeshPath,
		FStreamableDelegate::CreateUObject(
			this,
			&UPlayerAppearanceComponent::HandleAppearanceMeshLoaded,
			Slot,
			PartId
		)
	);
	if (!LoadHandle.IsValid())
	{
		PendingPartIdsBySlot.Remove(Slot);
		return false;
	}

	ActiveLoadHandles.Add(Slot, LoadHandle);
	return true;
}

void UPlayerAppearanceComponent::RefreshLeaderPose()
{
	for (const TPair<ERiftPlayerAppearanceSlot, TObjectPtr<USkeletalMeshComponent>>& MeshComponentPair : MeshComponentsBySlot)
	{
		if (USkeletalMeshComponent* MeshComponent = MeshComponentPair.Value.Get())
		{
			MeshComponent->SetLeaderPoseComponent(MasterMeshComponent);
		}
	}
}

void UPlayerAppearanceComponent::ClearAppearance()
{
	for (TPair<ERiftPlayerAppearanceSlot, TSharedPtr<FStreamableHandle>>& LoadHandlePair : ActiveLoadHandles)
	{
		if (LoadHandlePair.Value.IsValid())
		{
			LoadHandlePair.Value->CancelHandle();
		}
	}
	ActiveLoadHandles.Empty();
	PendingPartIdsBySlot.Empty();

	for (const TPair<ERiftPlayerAppearanceSlot, TObjectPtr<USkeletalMeshComponent>>& MeshComponentPair : MeshComponentsBySlot)
	{
		if (USkeletalMeshComponent* MeshComponent = MeshComponentPair.Value.Get())
		{
			MeshComponent->SetSkeletalMesh(nullptr);
		}
	}
}

USkeletalMeshComponent* UPlayerAppearanceComponent::GetComponentForSlot(
	const ERiftPlayerAppearanceSlot Slot) const
{
	const TObjectPtr<USkeletalMeshComponent>* MeshComponentPtr = MeshComponentsBySlot.Find(Slot);
	return MeshComponentPtr ? MeshComponentPtr->Get() : nullptr;
}

void UPlayerAppearanceComponent::CancelActiveLoadForSlot(const ERiftPlayerAppearanceSlot Slot)
{
	TSharedPtr<FStreamableHandle>* LoadHandlePtr = ActiveLoadHandles.Find(Slot);
	if (!LoadHandlePtr)
	{
		return;
	}

	if (LoadHandlePtr->IsValid())
	{
		(*LoadHandlePtr)->CancelHandle();
	}

	ActiveLoadHandles.Remove(Slot);
}

void UPlayerAppearanceComponent::HandleAppearanceMeshLoaded(
	const ERiftPlayerAppearanceSlot Slot,
	const FName PartId)
{
	const FName* PendingPartId = PendingPartIdsBySlot.Find(Slot);
	if (!PendingPartId || *PendingPartId != PartId)
	{
		return;
	}

	ActiveLoadHandles.Remove(Slot);

	USkeletalMeshComponent* TargetComponent = GetComponentForSlot(Slot);
	if (!TargetComponent || !AppearanceTable)
	{
		return;
	}

	const FRiftPlayerAppearancePartRow* Row =
		AppearanceTable->FindRow<FRiftPlayerAppearancePartRow>(PartId, TEXT(""), false);
	if (!Row || Row->Slot != Slot)
	{
		return;
	}

	USkeletalMesh* LoadedMesh = Row->SkeletalMesh.Get();
	if (!LoadedMesh)
	{
		return;
	}

	TargetComponent->SetSkeletalMesh(LoadedMesh);
}
