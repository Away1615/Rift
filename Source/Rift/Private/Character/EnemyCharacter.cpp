// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/EnemyCharacter.h"

#include "AbilitySystem/RiftAbilitySystemComponent.h"
#include "AbilitySystem/Attributes/RiftEnemyAttributeSet.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "Data/Enemy/Animation/EnemyAnimationConfig.h"
#include "Data/Enemy/EnemyCharacterConfig.h"
#include "Data/Enemy/Common/EnemyCommonConfig.h"
#include "Debug/Logger.h"
#include "Engine/World.h"

AEnemyCharacter::AEnemyCharacter()
{
	bReplicates = true;
	ACharacter::SetReplicateMovement(true);

	AbilitySystemComponent = CreateDefaultSubobject<URiftAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);
	AttributeSet = CreateDefaultSubobject<URiftEnemyAttributeSet>(TEXT("AttributeSet"));
}

void AEnemyCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	ApplyAnimationConfig();
}

void AEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();

	// The ASC is created in the constructor, so it must always exist on a properly
	// instantiated actor. A null here means a stale level instance placed before the
	// component was added to the class. Fix the asset by re-placing it.
	if (!AbilitySystemComponent)
	{
		FLogger::Error(
			this,
			FString::Printf(TEXT("%s has no AbilitySystemComponent. Re-place this actor in the level."), *GetName()),
			ELogSystem::Ability
		);
		return;
	}

	AbilitySystemComponent->InitAbilityActorInfo(this, this);
}

UAbilitySystemComponent* AEnemyCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AEnemyCharacter::HandlePoiseHit(const bool bPoiseBroken, const FVector& InstigatorLocation)
{
	if (!HasAuthority()) return;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(PoiseRegenTimerHandle);

		const UEnemyCommonConfig* CommonConfig = EnemyCharacterConfig ? EnemyCharacterConfig->EnemyCommonConfig : nullptr;
		const float PoiseRegenDelay = CommonConfig ? FMath::Max(0.0f, CommonConfig->PoiseRegenDelay) : 0.0f;
		if (PoiseRegenDelay > 0.0f)
		{
			World->GetTimerManager().SetTimer(
				PoiseRegenTimerHandle,
				this,
				&AEnemyCharacter::RestorePoise,
				PoiseRegenDelay,
				false
			);
		}
	}

	const ERiftHitReactDirection Direction = CalculateHitReactDirection(InstigatorLocation);
	Multicast_PlayHitReact(Direction, bPoiseBroken);
}

void AEnemyCharacter::Multicast_PlayHitReact_Implementation(const ERiftHitReactDirection Direction, const bool bPoiseBroken)
{
	const UEnemyAnimationConfig* AnimationConfig = EnemyCharacterConfig ? EnemyCharacterConfig->EnemyAnimationConfig : nullptr;
	if (!AnimationConfig) return;

	UAnimMontage* HitReactMontage = bPoiseBroken ? AnimationConfig->StaggerMontage : AnimationConfig->FlinchMontage;
	if (!HitReactMontage) return;

	USkeletalMeshComponent* CharacterMesh = GetMesh();
	if (!CharacterMesh) return;

	UAnimInstance* AnimInstance = CharacterMesh->GetAnimInstance();
	if (!AnimInstance) return;

	AnimInstance->Montage_Play(HitReactMontage);
	AnimInstance->Montage_JumpToSection(GetHitReactSectionName(Direction), HitReactMontage);
}

void AEnemyCharacter::ApplyAnimationConfig() const
{
	if (!EnemyCharacterConfig)
	{
		FLogger::Error(
			this,
			FString::Printf(TEXT("%s has no EnemyCharacterConfig."), *GetName()),
			ELogSystem::Character
		);
		return;
	}

	const UEnemyAnimationConfig* AnimationConfig = EnemyCharacterConfig->EnemyAnimationConfig;
	if (!AnimationConfig)
	{
		FLogger::Error(
			this,
			FString::Printf(TEXT("%s has no EnemyAnimationConfig."), *GetName()),
			ELogSystem::Animation
		);
		return;
	}

	USkeletalMeshComponent* CharacterMesh = GetMesh();
	if (!CharacterMesh) return;

	if (AnimationConfig->SkeletalMesh)
	{
		CharacterMesh->SetSkeletalMesh(AnimationConfig->SkeletalMesh);
	}

	if (AnimationConfig->AnimInstanceClass)
	{
		CharacterMesh->SetAnimInstanceClass(AnimationConfig->AnimInstanceClass);
	}
}

void AEnemyCharacter::RestorePoise()
{
	if (!HasAuthority() || !AttributeSet) return;

	AttributeSet->SetPoise(AttributeSet->GetMaxPoise());
}

ERiftHitReactDirection AEnemyCharacter::CalculateHitReactDirection(const FVector& InstigatorLocation) const
{
	FVector DirectionToInstigator = InstigatorLocation - GetActorLocation();
	DirectionToInstigator.Z = 0.0f;
	if (!DirectionToInstigator.Normalize())
	{
		return ERiftHitReactDirection::Front;
	}

	FVector Forward = GetActorForwardVector();
	Forward.Z = 0.0f;
	Forward.Normalize();

	FVector Right = GetActorRightVector();
	Right.Z = 0.0f;
	Right.Normalize();

	const float ForwardDot = FVector::DotProduct(Forward, DirectionToInstigator);
	const float RightDot = FVector::DotProduct(Right, DirectionToInstigator);

	if (FMath::Abs(ForwardDot) >= FMath::Abs(RightDot))
	{
		return ForwardDot >= 0.0f ? ERiftHitReactDirection::Front : ERiftHitReactDirection::Back;
	}

	return RightDot >= 0.0f ? ERiftHitReactDirection::Right : ERiftHitReactDirection::Left;
}

FName AEnemyCharacter::GetHitReactSectionName(const ERiftHitReactDirection Direction)
{
	switch (Direction)
	{
	case ERiftHitReactDirection::Front:
		return TEXT("Front");
	case ERiftHitReactDirection::Back:
		return TEXT("Back");
	case ERiftHitReactDirection::Left:
		return TEXT("Left");
	case ERiftHitReactDirection::Right:
		return TEXT("Right");
	default:
		return NAME_None;
	}
}
