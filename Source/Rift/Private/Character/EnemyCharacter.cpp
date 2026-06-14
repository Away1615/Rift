// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/EnemyCharacter.h"

#include "AbilitySystem/Effects/GE_EnemyMeleeDamage.h"
#include "AbilitySystem/RiftAbilitySystemComponent.h"
#include "AbilitySystem/Attributes/RiftEnemyAttributeSet.h"
#include "AbilitySystem/RiftGameplayTags.h"
#include "Character/PlayerCharacter.h"
#include "Animation/AnimInstance.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Data/Enemy/Animation/EnemyAnimationConfig.h"
#include "Data/Enemy/Combat/EnemyCombatConfig.h"
#include "Data/Enemy/EnemyCharacterConfig.h"
#include "Debug/Logger.h"
#include "Debug/RiftDebugCVars.h"
#include "DrawDebugHelpers.h"
#include "Engine/OverlapResult.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"

AEnemyCharacter::AEnemyCharacter()
{
	bReplicates = true;
	ACharacter::SetReplicateMovement(true);

	AbilitySystemComponent = CreateDefaultSubobject<URiftAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);
	AttributeSet = CreateDefaultSubobject<URiftEnemyAttributeSet>(TEXT("AttributeSet"));

	GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -87.578201f));
	GetMesh()->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));

	Head = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Head"));
	Head->SetupAttachment(GetMesh(), TEXT("head"));
	Head->SetRelativeRotation(FRotator(0.0f, 0.0f, -90.0f));
}

void AEnemyCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	ApplyAnimationConfig();
	ApplyWeaponsFromConfig();
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

	if (HasAuthority())
	{
		ApplyCommonAttributesFromConfig();
		GrantAbilities();
		GetWorldTimerManager().SetTimer(
			AttackDriverTimerHandle,
			this,
			&AEnemyCharacter::TryMeleeAttack,
			0.2f,
			true
		);
	}
}

void AEnemyCharacter::ApplyCommonAttributesFromConfig()
{
	if (!HasAuthority() || !AttributeSet || !EnemyCharacterConfig) return;

	AttributeSet->SetMaxHealth(EnemyCharacterConfig->MaxHealth);
	AttributeSet->SetHealth(EnemyCharacterConfig->Health);
	AttributeSet->SetMaxPoise(EnemyCharacterConfig->MaxPoise);
	AttributeSet->SetPoise(EnemyCharacterConfig->Poise);

	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->MaxWalkSpeed = EnemyCharacterConfig->MoveSpeed;
	}
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

		const float PoiseRegenDelay = EnemyCharacterConfig
			? FMath::Max(0.0f, EnemyCharacterConfig->PoiseRegenDelay)
			: 0.0f;
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
	if (bPoiseBroken)
	{
		const UEnemyCombatConfig* CombatConfig = EnemyCharacterConfig ? EnemyCharacterConfig->EnemyCombatConfig : nullptr;
		const float StaggerDuration = CombatConfig ? CombatConfig->PoiseBreakStaggerDuration : 1.2f;
		EnterStagger(Direction, StaggerDuration);
	}
	else
	{
		Multicast_PlayHitReact(Direction);
	}
}

void AEnemyCharacter::BeginAttackHitWindow()
{
	if (!HasAuthority()) return;

	HitPlayersThisAttack.Reset();
	PerfectDodgersThisAttack.Reset();
}

void AEnemyCharacter::TickAttackHitWindow()
{
	if (!HasAuthority()) return;

	UWorld* World = GetWorld();
	const UEnemyCombatConfig* CombatConfig = EnemyCharacterConfig ? EnemyCharacterConfig->EnemyCombatConfig : nullptr;
	if (!World || !CombatConfig || !AbilitySystemComponent) return;

	const FVector HitboxCenter = GetActorLocation() + GetActorForwardVector() * CombatConfig->HitboxForwardOffset;
	const float HitboxRadius = FMath::Max(0.0f, CombatConfig->HitboxRadius);
	if (HitboxRadius <= 0.0f) return;

	APlayerCharacter* StaggerDodger = nullptr;
	for (TActorIterator<APlayerCharacter> It(World); It; ++It)
	{
		APlayerCharacter* PlayerCharacter = *It;
		const TObjectKey<AActor> PlayerKey(PlayerCharacter);
		if (PerfectDodgersThisAttack.Contains(PlayerKey)) continue;
		if (!PlayerCharacter->IsPerfectDodgeWindowActive()) continue;

		const float CapsuleRadius = PlayerCharacter->GetCapsuleComponent()
			? PlayerCharacter->GetCapsuleComponent()->GetScaledCapsuleRadius()
			: 34.0f;
		const float Threshold = HitboxRadius + CapsuleRadius;
		if (FVector::DistSquared(PlayerCharacter->GetPerfectDodgeOrigin(), HitboxCenter) > FMath::Square(Threshold))
		{
			continue;
		}

		PerfectDodgersThisAttack.Add(PlayerKey);
		PlayerCharacter->HandlePerfectDodge(this);
		StaggerDodger = PlayerCharacter;
	}

	if (PerfectDodgersThisAttack.Num() == 0)
	{
		TArray<FOverlapResult> OverlapResults;
		FCollisionObjectQueryParams ObjectQueryParams;
		ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);
		FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(RiftEnemyMeleeHitWindow), false, this);
		QueryParams.AddIgnoredActor(this);

		World->OverlapMultiByObjectType(
			OverlapResults,
			HitboxCenter,
			FQuat::Identity,
			ObjectQueryParams,
			FCollisionShape::MakeSphere(HitboxRadius),
			QueryParams
		);

		for (const FOverlapResult& OverlapResult : OverlapResults)
		{
			APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(OverlapResult.GetActor());
			if (!PlayerCharacter) continue;

			const TObjectKey<AActor> PlayerKey(PlayerCharacter);
			if (HitPlayersThisAttack.Contains(PlayerKey)) continue;

			UAbilitySystemComponent* TargetAbilitySystemComponent = PlayerCharacter->GetAbilitySystemComponent();
			if (!TargetAbilitySystemComponent) continue;

			FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
			EffectContext.AddInstigator(this, this);
			EffectContext.AddSourceObject(this);

			FGameplayEffectSpecHandle DamageSpecHandle = AbilitySystemComponent->MakeOutgoingSpec(
				UGE_EnemyMeleeDamage::StaticClass(),
				1.0f,
				EffectContext
			);
			if (!DamageSpecHandle.IsValid()) continue;

			HitPlayersThisAttack.Add(PlayerKey);
			DamageSpecHandle.Data->SetSetByCallerMagnitude(
				RiftGameplayTags::SetByCaller_Damage,
				CombatConfig->AttackDamage
			);
			AbilitySystemComponent->ApplyGameplayEffectSpecToTarget(
				*DamageSpecHandle.Data.Get(),
				TargetAbilitySystemComponent
			);
		}
	}

	if (StaggerDodger)
	{
		ApplyPerfectDodgeStagger(StaggerDodger);
	}

	if (RiftDebugCVars::IsCombatDebugEnabled())
	{
		DrawDebugSphere(World, HitboxCenter, HitboxRadius, 16, FColor::Red, false, 0.1f, 0, 1.5f);
	}
}

void AEnemyCharacter::EndAttackHitWindow()
{
	if (!HasAuthority()) return;

	HitPlayersThisAttack.Reset();
	PerfectDodgersThisAttack.Reset();
}

void AEnemyCharacter::Multicast_PlayHitReact_Implementation(const ERiftHitReactDirection Direction)
{
	const UEnemyAnimationConfig* AnimationConfig = EnemyCharacterConfig ? EnemyCharacterConfig->EnemyAnimationConfig : nullptr;
	if (!AnimationConfig) return;

	UAnimMontage* HitReactMontage = AnimationConfig->FlinchMontage;
	if (!HitReactMontage) return;

	USkeletalMeshComponent* CharacterMesh = GetMesh();
	if (!CharacterMesh) return;

	UAnimInstance* AnimInstance = CharacterMesh->GetAnimInstance();
	if (!AnimInstance) return;

	AnimInstance->Montage_Play(HitReactMontage);
	AnimInstance->Montage_JumpToSection(GetHitReactSectionName(Direction), HitReactMontage);
}

void AEnemyCharacter::Multicast_PlayStagger_Implementation(const ERiftHitReactDirection Direction)
{
	const UEnemyAnimationConfig* AnimationConfig = EnemyCharacterConfig ? EnemyCharacterConfig->EnemyAnimationConfig : nullptr;
	if (!AnimationConfig) return;

	UAnimMontage* StaggerMontage = AnimationConfig->StaggerMontage;
	if (!StaggerMontage) return;

	USkeletalMeshComponent* CharacterMesh = GetMesh();
	if (!CharacterMesh) return;

	UAnimInstance* AnimInstance = CharacterMesh->GetAnimInstance();
	if (!AnimInstance) return;

	AnimInstance->Montage_Play(StaggerMontage);
	AnimInstance->Montage_JumpToSection(GetHitReactSectionName(Direction), StaggerMontage);
}

void AEnemyCharacter::Multicast_ExitStagger_Implementation()
{
	const UEnemyAnimationConfig* AnimationConfig = EnemyCharacterConfig ? EnemyCharacterConfig->EnemyAnimationConfig : nullptr;
	if (!AnimationConfig) return;

	UAnimMontage* StaggerMontage = AnimationConfig->StaggerMontage;
	if (!StaggerMontage) return;

	USkeletalMeshComponent* CharacterMesh = GetMesh();
	if (!CharacterMesh) return;

	UAnimInstance* AnimInstance = CharacterMesh->GetAnimInstance();
	if (!AnimInstance || !AnimInstance->Montage_IsPlaying(StaggerMontage)) return;

	AnimInstance->Montage_JumpToSection(TEXT("End"), StaggerMontage);
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

	if (AnimationConfig->HeadMesh && Head)
	{
		Head->SetStaticMesh(AnimationConfig->HeadMesh);
	}

	if (AnimationConfig->AnimInstanceClass)
	{
		CharacterMesh->SetAnimInstanceClass(AnimationConfig->AnimInstanceClass);
	}
}

void AEnemyCharacter::ApplyWeaponsFromConfig()
{
	if (!EnemyCharacterConfig) return;
	ApplyWeapons(EnemyCharacterConfig->Weapons);
}

void AEnemyCharacter::GrantAbilities()
{
	if (!HasAuthority() || !EnemyCharacterConfig || !AbilitySystemComponent) return;

	for (const TSubclassOf<UGameplayAbility>& AbilityClass : EnemyCharacterConfig->GrantedAbilities)
	{
		if (!AbilityClass) continue;

		AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(AbilityClass));
	}
}

void AEnemyCharacter::TryMeleeAttack()
{
	if (!HasAuthority()) return;

	UWorld* World = GetWorld();
	const UEnemyCombatConfig* CombatConfig = EnemyCharacterConfig ? EnemyCharacterConfig->EnemyCombatConfig : nullptr;
	if (!World || !CombatConfig || !AbilitySystemComponent) return;

	APlayerCharacter* ClosestPlayer = nullptr;
	float ClosestDistanceSq = FMath::Square(CombatConfig->AttackRange);
	const FVector EnemyLocation = GetActorLocation();

	for (TActorIterator<APlayerCharacter> It(World); It; ++It)
	{
		APlayerCharacter* PlayerCharacter = *It;
		FVector ToPlayer = PlayerCharacter->GetActorLocation() - EnemyLocation;
		ToPlayer.Z = 0.0f;

		const float DistanceSq = ToPlayer.SizeSquared();
		if (DistanceSq <= ClosestDistanceSq)
		{
			ClosestDistanceSq = DistanceSq;
			ClosestPlayer = PlayerCharacter;
		}
	}

	if (!ClosestPlayer) return;
	if (AbilitySystemComponent->HasMatchingGameplayTag(RiftGameplayTags::State_Attacking) ||
		AbilitySystemComponent->HasMatchingGameplayTag(RiftGameplayTags::State_HitReact))
	{
		return;
	}

	FVector ToTarget = ClosestPlayer->GetActorLocation() - EnemyLocation;
	ToTarget.Z = 0.0f;
	if (ToTarget.Normalize())
	{
		SetActorRotation(ToTarget.ToOrientationRotator());
	}

	const float Now = World->GetTimeSeconds();
	if (Now < NextAttackTime) return;

	FGameplayTagContainer AttackTags;
	AttackTags.AddTag(RiftGameplayTags::Ability_Enemy_MeleeAttack);
	if (AbilitySystemComponent->TryActivateAbilitiesByTag(AttackTags))
	{
		NextAttackTime = Now + CombatConfig->AttackCooldown;
	}
}

void AEnemyCharacter::ApplyPerfectDodgeStagger(APlayerCharacter* Dodger)
{
	if (!HasAuthority()) return;

	const UEnemyCombatConfig* Config = EnemyCharacterConfig ? EnemyCharacterConfig->EnemyCombatConfig : nullptr;
	const float Dilation = Config
		? FMath::Clamp(Config->PerfectDodgeTimeDilation, 0.01f, 1.0f)
		: 0.3f;
	const float Duration = Config ? FMath::Max(0.1f, Config->PerfectDodgeStaggerDuration) : 2.0f;

	CustomTimeDilation = Dilation;

	const FVector DodgerLocation = Dodger
		? Dodger->GetActorLocation()
		: GetActorLocation() + GetActorForwardVector();
	EnterStagger(CalculateHitReactDirection(DodgerLocation), Duration);
}

void AEnemyCharacter::EnterStagger(const ERiftHitReactDirection Direction, const float Duration)
{
	if (!HasAuthority() || !AbilitySystemComponent) return;

	FGameplayTagContainer AttackTags;
	AttackTags.AddTag(RiftGameplayTags::Ability_Enemy_MeleeAttack);
	AbilitySystemComponent->CancelAbilities(&AttackTags);

	AbilitySystemComponent->SetLooseGameplayTagCount(RiftGameplayTags::State_HitReact, 1);

	Multicast_PlayStagger(Direction);

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(StaggerTimerHandle);
		World->GetTimerManager().SetTimer(
			StaggerTimerHandle,
			this,
			&AEnemyCharacter::ExitStagger,
			FMath::Max(0.1f, Duration),
			false
		);
	}
}

void AEnemyCharacter::ExitStagger()
{
	if (!HasAuthority()) return;

	Multicast_ExitStagger();

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->SetLooseGameplayTagCount(RiftGameplayTags::State_HitReact, 0);
	}

	CustomTimeDilation = 1.0f;
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
