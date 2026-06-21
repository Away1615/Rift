#include "UI/Lobby/RiftLobbyPreviewCharacter.h"

#include "Appearance/PlayerAppearanceComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Data/Player/PlayerClassConfig.h"

ARiftLobbyPreviewCharacter::ARiftLobbyPreviewCharacter()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = false;
	SetReplicateMovement(false);

	SceneRootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRootComponent);

	PreviewPivotComponent = CreateDefaultSubobject<USceneComponent>(TEXT("PreviewPivot"));
	PreviewPivotComponent->SetupAttachment(SceneRootComponent);

	PreviewMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("PreviewMesh"));
	PreviewMeshComponent->SetupAttachment(PreviewPivotComponent);
	PreviewMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	HairMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("HairMesh"));
	HairMeshComponent->SetupAttachment(PreviewMeshComponent);
	HairMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ArmUpperLeftMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ArmUpperLeftMesh"));
	ArmUpperLeftMeshComponent->SetupAttachment(PreviewMeshComponent);
	ArmUpperLeftMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ArmUpperRightMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ArmUpperRightMesh"));
	ArmUpperRightMeshComponent->SetupAttachment(PreviewMeshComponent);
	ArmUpperRightMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	AppearanceComponent = CreateDefaultSubobject<UPlayerAppearanceComponent>(TEXT("AppearanceComponent"));
	AppearanceComponent->SetMasterMeshComponent(PreviewMeshComponent);
	AppearanceComponent->RegisterAppearanceMeshComponent(ERiftPlayerAppearanceSlot::Hair, HairMeshComponent);
	AppearanceComponent->RegisterAppearanceMeshComponent(ERiftPlayerAppearanceSlot::ArmUpperLeft, ArmUpperLeftMeshComponent);
	AppearanceComponent->RegisterAppearanceMeshComponent(ERiftPlayerAppearanceSlot::ArmUpperRight, ArmUpperRightMeshComponent);
	AppearanceComponent->RefreshLeaderPose();
}

void ARiftLobbyPreviewCharacter::ApplyPreviewClass(UPlayerClassConfig* PreviewClassConfig)
{
	if (!PreviewClassConfig || !PreviewClassConfig->AnimClass || !PreviewMeshComponent)
	{
		return;
	}

	PreviewMeshComponent->SetAnimInstanceClass(PreviewClassConfig->AnimClass);
	RefreshAppearanceLeaderPose();
}

bool ARiftLobbyPreviewCharacter::ApplyPreviewAppearance(const FRiftPlayerAppearanceSelection& AppearanceSelection)
{
	if (!AppearanceComponent)
	{
		return false;
	}

	SyncAppearanceTable();
	return AppearanceComponent->ApplyAppearanceSelection(AppearanceSelection);
}

void ARiftLobbyPreviewCharacter::ClearPreviewAppearance()
{
	if (AppearanceComponent)
	{
		AppearanceComponent->ClearAppearance();
	}
}

void ARiftLobbyPreviewCharacter::RefreshAppearanceLeaderPose()
{
	if (AppearanceComponent)
	{
		AppearanceComponent->RefreshLeaderPose();
	}
}

void ARiftLobbyPreviewCharacter::SyncAppearanceTable()
{
	if (AppearanceComponent && AppearanceTable)
	{
		AppearanceComponent->AppearanceTable = AppearanceTable;
	}
}
