#pragma once

#include "CoreMinimal.h"
#include "Appearance/RiftPlayerAppearanceTypes.h"
#include "Components/ActorComponent.h"
#include "PlayerAppearanceComponent.generated.h"

class UDataTable;
class USkeletalMeshComponent;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class RIFT_API UPlayerAppearanceComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPlayerAppearanceComponent();

	UFUNCTION(BlueprintCallable, Category="Appearance")
	void SetMasterMeshComponent(USkeletalMeshComponent* InMasterMeshComponent);

	UFUNCTION(BlueprintCallable, Category="Appearance")
	void RegisterAppearanceMeshComponent(ERiftPlayerAppearanceSlot Slot, USkeletalMeshComponent* MeshComponent);

	UFUNCTION(BlueprintCallable, Category="Appearance")
	void UnregisterAppearanceMeshComponent(ERiftPlayerAppearanceSlot Slot);

	UFUNCTION(BlueprintCallable, Category="Appearance")
	bool ApplyAppearanceSelection(const FRiftPlayerAppearanceSelection& Selection);

	UFUNCTION(BlueprintCallable, Category="Appearance")
	bool ApplyAppearancePart(ERiftPlayerAppearanceSlot Slot, FName PartId);

	UFUNCTION(BlueprintCallable, Category="Appearance")
	void RefreshLeaderPose();

	UFUNCTION(BlueprintCallable, Category="Appearance")
	void ClearAppearance();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Appearance")
	TObjectPtr<UDataTable> AppearanceTable;

private:
	USkeletalMeshComponent* GetComponentForSlot(ERiftPlayerAppearanceSlot Slot) const;
	void CancelActiveLoadForSlot(ERiftPlayerAppearanceSlot Slot);
	void HandleAppearanceMeshLoaded(ERiftPlayerAppearanceSlot Slot, FName PartId);

	UPROPERTY(Transient)
	TObjectPtr<USkeletalMeshComponent> MasterMeshComponent;

	UPROPERTY(Transient)
	TMap<ERiftPlayerAppearanceSlot, TObjectPtr<USkeletalMeshComponent>> MeshComponentsBySlot;

	TMap<ERiftPlayerAppearanceSlot, TSharedPtr<struct FStreamableHandle>> ActiveLoadHandles;
	TMap<ERiftPlayerAppearanceSlot, FName> PendingPartIdsBySlot;
};
