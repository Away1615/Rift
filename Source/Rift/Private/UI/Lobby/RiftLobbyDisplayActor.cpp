// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Lobby/RiftLobbyDisplayActor.h"

#include "Animation/AnimationAsset.h"
#include "Appearance/PlayerAppearanceComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Data/Player/PlayerClassConfig.h"
#include "Player/BasePlayerState.h"

ARiftLobbyDisplayActor::ARiftLobbyDisplayActor()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Scene"));
	SetRootComponent(SceneRootComponent);

	PreviewPivotComponent = CreateDefaultSubobject<USceneComponent>(TEXT("PreviewPivot"));
	PreviewPivotComponent->SetupAttachment(SceneRootComponent);
	PreviewPivotComponent->SetRelativeRotation(FRotator(0.0, DefaultPreviewYaw, 0.0));

	PreviewMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("PreviewMesh"));
	PreviewMeshComponent->SetupAttachment(PreviewPivotComponent);
	PreviewMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PreviewMeshComponent->SetVisibility(false, true);
	PreviewMeshComponent->SetHiddenInGame(true, true);

	HairMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("HairMesh"));
	HairMeshComponent->SetupAttachment(PreviewMeshComponent);
	HairMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ArmUpperLeftMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ArmUpperLeftMesh"));
	ArmUpperLeftMeshComponent->SetupAttachment(PreviewMeshComponent);
	ArmUpperLeftMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ArmUpperRightMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ArmUpperRightMesh"));
	ArmUpperRightMeshComponent->SetupAttachment(PreviewMeshComponent);
	ArmUpperRightMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	GhostMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("GhostMesh"));
	GhostMeshComponent->SetupAttachment(PreviewPivotComponent);
	GhostMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GhostMeshComponent->SetVisibility(false, true);
	GhostMeshComponent->SetHiddenInGame(true, true);

	AppearanceComponent = CreateDefaultSubobject<UPlayerAppearanceComponent>(TEXT("AppearanceComponent"));
	AppearanceComponent->SetMasterMeshComponent(PreviewMeshComponent);
	AppearanceComponent->RegisterAppearanceMeshComponent(ERiftPlayerAppearanceSlot::Hair, HairMeshComponent);
	AppearanceComponent->RegisterAppearanceMeshComponent(ERiftPlayerAppearanceSlot::ArmUpperLeft, ArmUpperLeftMeshComponent);
	AppearanceComponent->RegisterAppearanceMeshComponent(ERiftPlayerAppearanceSlot::ArmUpperRight, ArmUpperRightMeshComponent);
}

void ARiftLobbyDisplayActor::SetLobbySlotIndex(const int32 NewLobbySlotIndex)
{
	LobbySlotIndex = NewLobbySlotIndex;
}

void ARiftLobbyDisplayActor::RefreshFromLobbyData(
	ABasePlayerState* PlayerState,
	const ERiftLobbyDisplayMode DisplayMode,
	const FRiftPlayerAppearanceSelection& AppearanceSelection
)
{
	CurrentPlayerState = PlayerState;
	CurrentDisplayMode = DisplayMode;
	CurrentAppearanceSelection = AppearanceSelection;

	OnLobbyDisplayRefreshed(PlayerState, DisplayMode, AppearanceSelection);

	switch (DisplayMode)
	{
	case ERiftLobbyDisplayMode::Empty:
		OnDisplayEmpty();
		break;
	case ERiftLobbyDisplayMode::Ghost:
		OnDisplayGhost(PlayerState);
		break;
	case ERiftLobbyDisplayMode::LocalPreview:
		OnDisplayLocalPreview(PlayerState, AppearanceSelection);
		break;
	case ERiftLobbyDisplayMode::Confirmed:
		OnDisplayConfirmed(PlayerState, AppearanceSelection);
		break;
	default:
		OnDisplayEmpty();
		break;
	}

	ApplyNativeDisplayMode(PlayerState, DisplayMode, AppearanceSelection);
}

void ARiftLobbyDisplayActor::SetPreviewRotationComponent(USceneComponent* NewPreviewRotationComponent)
{
	PreviewRotationComponent = NewPreviewRotationComponent;
}

void ARiftLobbyDisplayActor::AddPreviewYaw(const float DeltaYaw)
{
	SetPreviewYaw(PreviewYaw + DeltaYaw);
}

void ARiftLobbyDisplayActor::SetPreviewYaw(const float NewPreviewYaw)
{
	PreviewYaw = NewPreviewYaw;

	USceneComponent* RotationComponent = GetPreviewRotationComponent();
	if (!RotationComponent)
	{
		return;
	}

	FRotator Rotation = RotationComponent->GetRelativeRotation();
	Rotation.Yaw = PreviewYaw;
	RotationComponent->SetRelativeRotation(Rotation);
}

void ARiftLobbyDisplayActor::ResetPreviewYaw()
{
	SetPreviewYaw(DefaultPreviewYaw);
}

USceneComponent* ARiftLobbyDisplayActor::GetPreviewRotationComponent() const
{
	if (PreviewRotationComponent)
	{
		return PreviewRotationComponent;
	}

	if (PreviewPivotComponent)
	{
		return PreviewPivotComponent;
	}

	return GetRootComponent();
}

void ARiftLobbyDisplayActor::ApplyNativeDisplayMode(
	ABasePlayerState* PlayerState,
	const ERiftLobbyDisplayMode DisplayMode,
	const FRiftPlayerAppearanceSelection& AppearanceSelection
)
{
	SetActorHiddenInGame(false);

	switch (DisplayMode)
	{
	case ERiftLobbyDisplayMode::Empty:
		StopGhostAnimation();
		SetGhostVisible(false);
		SetPreviewVisible(false);
		break;
	case ERiftLobbyDisplayMode::Ghost:
		SetPreviewVisible(false);
		SetGhostVisible(true);
		PlayGhostAnimation();
		break;
	case ERiftLobbyDisplayMode::LocalPreview:
	case ERiftLobbyDisplayMode::Confirmed:
		StopGhostAnimation();
		SetGhostVisible(false);
		if (PlayerState)
		{
			ApplyPreviewClass(PlayerState->GetSelectedPlayerClassConfig());
			ApplyPreviewAppearance(AppearanceSelection);
		}
		SetPreviewVisible(true);
		break;
	default:
		StopGhostAnimation();
		SetGhostVisible(false);
		SetPreviewVisible(false);
		break;
	}
}

void ARiftLobbyDisplayActor::SetGhostVisible(const bool bVisible)
{
	if (GhostMeshComponent)
	{
		GhostMeshComponent->SetVisibility(bVisible, true);
	}
}

void ARiftLobbyDisplayActor::SetPreviewVisible(const bool bVisible)
{
	if (!PreviewMeshComponent)
	{
		return;
	}

	PreviewMeshComponent->SetVisibility(bVisible, true);
	PreviewMeshComponent->SetHiddenInGame(!bVisible, true);
}

void ARiftLobbyDisplayActor::PlayGhostAnimation()
{
	if (!GhostMeshComponent || GhostAnimations.IsEmpty())
	{
		return;
	}

	const int32 AnimationIndex = FMath::RandRange(0, GhostAnimations.Num() - 1);
	UAnimationAsset* GhostAnimation = GhostAnimations[AnimationIndex];
	if (!GhostAnimation)
	{
		return;
	}

	GhostMeshComponent->PlayAnimation(GhostAnimation, true);
}

void ARiftLobbyDisplayActor::StopGhostAnimation()
{
	if (GhostMeshComponent)
	{
		GhostMeshComponent->Stop();
	}
}

void ARiftLobbyDisplayActor::ApplyPreviewClass(UPlayerClassConfig* PreviewClassConfig)
{
	if (!PreviewClassConfig || !PreviewClassConfig->AnimClass || !PreviewMeshComponent)
	{
		return;
	}

	PreviewMeshComponent->SetAnimInstanceClass(PreviewClassConfig->AnimClass);
	RefreshAppearanceLeaderPose();
}

void ARiftLobbyDisplayActor::ApplyPreviewAppearance(const FRiftPlayerAppearanceSelection& AppearanceSelection)
{
	if (!AppearanceComponent)
	{
		return;
	}

	AppearanceComponent->ApplyAppearanceSelection(AppearanceSelection);
}

void ARiftLobbyDisplayActor::RefreshAppearanceLeaderPose()
{
	if (AppearanceComponent)
	{
		AppearanceComponent->RefreshLeaderPose();
	}
}
