// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/EnemyCharacter.h"

#include "AI/RiftEnemyAIController.h"
#include "AbilitySystem/Effects/GE_EnemyMeleeDamage.h"
#include "AbilitySystem/Effects/GE_GainResource.h"
#include "AbilitySystem/RiftAbilitySystemComponent.h"
#include "AbilitySystem/Attributes/RiftEnemyAttributeSet.h"
#include "AbilitySystem/Attributes/RiftPlayerAttributeSet.h"
#include "AbilitySystem/RiftGameplayTags.h"
#include "AbilitySystemInterface.h"
#include "Character/PlayerCharacter.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Data/Enemy/Animation/EnemyAnimationConfig.h"
#include "Data/Enemy/Combat/EnemyCombatConfig.h"
#include "Data/Enemy/EnemyCharacterConfig.h"
#include "Debug/Logger.h"
#include "Debug/RiftDebugCVars.h"
#include "DrawDebugHelpers.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"
#include "GameFramework/Controller.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "UI/Enemy/EnemyHealthBarWidget.h"

AEnemyCharacter::AEnemyCharacter()
{
	bReplicates = true;
	ACharacter::SetReplicateMovement(true);
	AIControllerClass = ARiftEnemyAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	bUseControllerRotationYaw = false;

	AbilitySystemComponent = CreateDefaultSubobject<URiftAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);
	AttributeSet = CreateDefaultSubobject<URiftEnemyAttributeSet>(TEXT("AttributeSet"));

	GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -87.578201f));
	GetMesh()->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));

	Head = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Head"));
	Head->SetupAttachment(GetMesh(), TEXT("head"));
	Head->SetRelativeRotation(FRotator(0.0f, 0.0f, -90.0f));

	HealthBarWidgetComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthBarWidget"));
	HealthBarWidgetComp->SetupAttachment(GetRootComponent());
	HealthBarWidgetComp->SetWidgetSpace(EWidgetSpace::Screen);
	HealthBarWidgetComp->SetDrawAtDesiredSize(true);
	HealthBarWidgetComp->SetRelativeLocation(FVector(0.0f, 0.0f, 110.0f));
	HealthBarWidgetComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->bOrientRotationToMovement = true;
	}
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
		StartSpawnIntroOrAI();
	}

	if (HealthBarWidgetComp)
	{
		if (!HealthBarWidgetComp->GetWidgetClass() && HealthBarWidgetClass)
		{
			HealthBarWidgetComp->SetWidgetClass(HealthBarWidgetClass);
		}

		HealthBarWidgetComp->InitWidget();
		if (UEnemyHealthBarWidget* HealthBarWidget = Cast<UEnemyHealthBarWidget>(HealthBarWidgetComp->GetUserWidgetObject()))
		{
			HealthBarWidget->InitializeFor(AbilitySystemComponent);
		}
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

const UEnemyCombatConfig* AEnemyCharacter::GetEnemyCombatConfig() const
{
	return EnemyCharacterConfig ? EnemyCharacterConfig->EnemyCombatConfig : nullptr;
}

bool AEnemyCharacter::IsStaggeredForAnimation() const
{
	return AbilitySystemComponent &&
		AbilitySystemComponent->HasMatchingGameplayTag(RiftGameplayTags::State_Staggered);
}

bool AEnemyCharacter::IsDeadForAnimation() const
{
	return bIsDead;
}

bool AEnemyCharacter::IsBlocking() const
{
	return AbilitySystemComponent &&
		AbilitySystemComponent->HasMatchingGameplayTag(RiftGameplayTags::State_Blocking);
}

bool AEnemyCharacter::IsStaggeredForAI() const
{
	return IsStaggeredForAnimation();
}

bool AEnemyCharacter::IsAttackingForAI() const
{
	return AbilitySystemComponent &&
		AbilitySystemComponent->HasMatchingGameplayTag(RiftGameplayTags::State_Attacking);
}

bool AEnemyCharacter::IsIntroForAI() const
{
	return AbilitySystemComponent &&
		AbilitySystemComponent->HasMatchingGameplayTag(RiftGameplayTags::State_Intro);
}

bool AEnemyCharacter::IsDiscoveringForAI() const
{
	return AbilitySystemComponent &&
		AbilitySystemComponent->HasMatchingGameplayTag(RiftGameplayTags::State_Discovering);
}

bool AEnemyCharacter::CanStartMeleeAttack(AActor* TargetActor) const
{
	const UEnemyCombatConfig* CombatConfig = GetEnemyCombatConfig();
	const UWorld* World = GetWorld();
	if (!HasAuthority() ||
		bIsDead ||
		IsIntroForAI() ||
		IsDiscoveringForAI() ||
		!AbilitySystemComponent ||
		!CombatConfig ||
		!TargetActor ||
		!World)
	{
		return false;
	}

	const APlayerCharacter* PlayerTarget = Cast<APlayerCharacter>(TargetActor);
	if (PlayerTarget && PlayerTarget->IsDead()) return false;

	FVector ToTarget = TargetActor->GetActorLocation() - GetActorLocation();
	ToTarget.Z = 0.0f;
	if (ToTarget.SizeSquared() > FMath::Square(CombatConfig->AttackRange)) return false;
	if (World->GetTimeSeconds() < NextAttackTime) return false;

	return HasAuthority() &&
		!bIsDead &&
		AbilitySystemComponent &&
		!IsStaggeredForAI() &&
		!IsIntroForAI() &&
		!IsDiscoveringForAI() &&
		!IsAttackingForAI();
}

bool AEnemyCharacter::TryStartMeleeAttack(AActor* TargetActor)
{
	if (!CanStartMeleeAttack(TargetActor)) return false;

	FVector ToTarget = TargetActor->GetActorLocation() - GetActorLocation();
	ToTarget.Z = 0.0f;
	if (ToTarget.Normalize())
	{
		SetActorRotation(ToTarget.ToOrientationRotator());
	}

	FGameplayTagContainer AttackTags;
	AttackTags.AddTag(RiftGameplayTags::Ability_Enemy_MeleeAttack);
	const bool bActivated = AbilitySystemComponent->TryActivateAbilitiesByTag(AttackTags);
	if (bActivated)
	{
		const UEnemyCombatConfig* CombatConfig = GetEnemyCombatConfig();
		const UWorld* World = GetWorld();
		if (CombatConfig && World)
		{
			NextAttackTime = World->GetTimeSeconds() + CombatConfig->AttackCooldown;
		}
	}

	return bActivated;
}

bool AEnemyCharacter::CanStartShieldBlock(AActor* TargetActor) const
{
	const UEnemyCombatConfig* CombatConfig = GetEnemyCombatConfig();
	const UWorld* World = GetWorld();
	if (!HasAuthority() ||
		!TargetActor ||
		bIsDead ||
		IsStaggeredForAI() ||
		IsIntroForAI() ||
		IsDiscoveringForAI() ||
		IsBlocking() ||
		!CombatConfig ||
		!CombatConfig->ShieldBlockMontage ||
		!AbilitySystemComponent ||
		!World)
	{
		return false;
	}

	const float ShieldBlockCooldown = FMath::Max(0.0f, CombatConfig->ShieldBlockCooldown);
	if (World->GetTimeSeconds() < LastShieldBlockTime + ShieldBlockCooldown)
	{
		return false;
	}

	return true;
}

bool AEnemyCharacter::TryStartShieldBlock(AActor* TargetActor)
{
	if (!CanStartShieldBlock(TargetActor)) return false;

	FGameplayTagContainer ShieldBlockTags;
	ShieldBlockTags.AddTag(RiftGameplayTags::Ability_Enemy_ShieldBlock);
	const bool bActivated = AbilitySystemComponent->TryActivateAbilitiesByTag(ShieldBlockTags);
	if (bActivated)
	{
		if (const UWorld* World = GetWorld())
		{
			LastShieldBlockTime = World->GetTimeSeconds();
		}
	}

	return bActivated;
}

bool AEnemyCharacter::CanPlayDiscoverReaction(AActor* TargetActor) const
{
	return HasAuthority() &&
		TargetActor &&
		!bIsDead &&
		!bHasPlayedDiscoverReaction &&
		!IsStaggeredForAI() &&
		!IsIntroForAI() &&
		!IsDiscoveringForAI();
}

void AEnemyCharacter::TryPlayDiscoverReaction(AActor* TargetActor)
{
	if (!CanPlayDiscoverReaction(TargetActor)) return;

	bHasPlayedDiscoverReaction = true;

	const UEnemyAnimationConfig* AnimationConfig = EnemyCharacterConfig ? EnemyCharacterConfig->EnemyAnimationConfig : nullptr;
	UAnimMontage* DiscoverMontage = AnimationConfig ? AnimationConfig->DiscoverMontage : nullptr;
	if (!DiscoverMontage) return;

	SetDiscoveringState(true);
	StopAIMovement();
	Multicast_PlayDiscover();

	const float DiscoverDuration = DiscoverMontage->GetPlayLength();
	if (DiscoverDuration <= 0.0f)
	{
		FinishDiscoverReaction();
		return;
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(DiscoverTimerHandle);
		World->GetTimerManager().SetTimer(
			DiscoverTimerHandle,
			this,
			&AEnemyCharacter::FinishDiscoverReaction,
			DiscoverDuration,
			false
		);
	}
}

void AEnemyCharacter::HandlePoiseHit(const bool bPoiseBroken, const FVector& InstigatorLocation)
{
	if (!HasAuthority() || bIsDead) return;

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

	if (bPoiseBroken)
	{
		const UEnemyCombatConfig* CombatConfig = EnemyCharacterConfig ? EnemyCharacterConfig->EnemyCombatConfig : nullptr;
		const float StaggeredDuration = CombatConfig ? CombatConfig->PoiseBreakStaggerDuration : 1.2f;
		EnterStaggered(StaggeredDuration);
	}
	else
	{
		const ERiftHitReactDirection Direction = CalculateHitReactDirection(this, InstigatorLocation);
		Multicast_PlayHit(Direction);
	}
}

void AEnemyCharacter::BeginAttackHitWindow()
{
	if (!HasAuthority() || bIsDead) return;

	HitPlayersThisAttack.Reset();
}

void AEnemyCharacter::TickAttackHitWindow()
{
	if (!HasAuthority() || bIsDead) return;

	UWorld* World = GetWorld();
	const UEnemyCombatConfig* CombatConfig = EnemyCharacterConfig ? EnemyCharacterConfig->EnemyCombatConfig : nullptr;
	if (!World || !CombatConfig || !AbilitySystemComponent) return;

	const FVector HitboxCenter = GetActorLocation() + GetActorForwardVector() * CombatConfig->HitboxForwardOffset;
	const float HitboxRadius = FMath::Max(0.0f, CombatConfig->HitboxRadius);
	if (HitboxRadius <= 0.0f) return;

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
		if (PlayerCharacter->IsDead()) continue;

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
		float PlayerPoiseDamage = CombatConfig->AttackPoiseDamage;
		if (CombatConfig->bForcePlayerPoiseBreak)
		{
			const float CurrentPoise = TargetAbilitySystemComponent->GetNumericAttribute(
				URiftPlayerAttributeSet::GetPoiseAttribute()
			);
			PlayerPoiseDamage = FMath::Max(PlayerPoiseDamage, CurrentPoise);
			if (PlayerPoiseDamage <= 0.0f)
			{
				PlayerPoiseDamage = 1.0f;
			}
		}
		DamageSpecHandle.Data->SetSetByCallerMagnitude(
			RiftGameplayTags::SetByCaller_PoiseDamage,
			PlayerPoiseDamage
		);
		AbilitySystemComponent->ApplyGameplayEffectSpecToTarget(
			*DamageSpecHandle.Data.Get(),
			TargetAbilitySystemComponent
		);

		FVector HitNormal = GetActorLocation() - PlayerCharacter->GetActorLocation();
		HitNormal.Z = 0.0f;
		if (!HitNormal.Normalize())
		{
			HitNormal = -GetActorForwardVector();
		}

		FGameplayCueParameters CueParameters;
		CueParameters.Location = PlayerCharacter->GetActorLocation();
		CueParameters.Normal = HitNormal;
		CueParameters.Instigator = this;
		CueParameters.EffectCauser = this;
		TargetAbilitySystemComponent->ExecuteGameplayCue(
			RiftGameplayTags::GameplayCue_Combat_PlayerHit,
			CueParameters
		);

		PlayerCharacter->HandleHitFeedback(
			CombatConfig->PlayerHitFeedbackPolicy,
			this,
			CombatConfig->AttackDamage
		);
	}

	if (RiftDebugCVars::IsCombatDebugEnabled())
	{
		DrawDebugSphere(World, HitboxCenter, HitboxRadius, 16, FColor::Red, false, 0.1f, 0, 1.5f);
	}
}

void AEnemyCharacter::EndAttackHitWindow()
{
	if (!HasAuthority() || bIsDead) return;

	HitPlayersThisAttack.Reset();
}

void AEnemyCharacter::Multicast_PlayHit_Implementation(const ERiftHitReactDirection Direction)
{
	LastHitReactDirection = Direction;

	const UEnemyAnimationConfig* AnimationConfig = EnemyCharacterConfig ? EnemyCharacterConfig->EnemyAnimationConfig : nullptr;
	if (!AnimationConfig) return;

	UAnimMontage* HitMontage = AnimationConfig->HitMontage;
	if (!HitMontage) return;

	USkeletalMeshComponent* CharacterMesh = GetMesh();
	if (!CharacterMesh) return;

	UAnimInstance* AnimInstance = CharacterMesh->GetAnimInstance();
	if (!AnimInstance) return;

	AnimInstance->Montage_Play(HitMontage);
	AnimInstance->Montage_JumpToSection(GetHitReactSectionName(Direction), HitMontage);
}

void AEnemyCharacter::Multicast_PlaySpawnIntro_Implementation()
{
	PlaySpawnIntroMontage();
}

void AEnemyCharacter::Multicast_PlayDiscover_Implementation()
{
	PlayDiscoverMontage();
}

void AEnemyCharacter::HandleDeath(AActor* Killer)
{
	if (bIsDead || !HasAuthority()) return;

	bIsDead = true;
	SetBlockingState(false);
	SetIntroState(false);
	SetDiscoveringState(false);

	if (HealthBarWidgetComp)
	{
		HealthBarWidgetComp->SetVisibility(false);
	}

	GetWorldTimerManager().ClearTimer(StaggeredTimerHandle);
	GetWorldTimerManager().ClearTimer(SpawnIntroTimerHandle);
	GetWorldTimerManager().ClearTimer(DiscoverTimerHandle);
	GetWorldTimerManager().ClearTimer(PoiseRegenTimerHandle);
	HitPlayersThisAttack.Reset();
	CustomTimeDilation = 1.0f;

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->CancelAllAbilities();
		SetStaggeredState(false);
		AbilitySystemComponent->AddLooseGameplayTag(RiftGameplayTags::State_Dead);
	}

	SetActorEnableCollision(false);
	StopAIMovement();
	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->DisableMovement();
	}

	GrantKillReward(Killer);

	const ERiftHitReactDirection Direction = Killer
		? CalculateHitReactDirection(this, Killer->GetActorLocation())
		: ERiftHitReactDirection::Front;
	Multicast_PlayDeath(Direction);

	const float DespawnDelay = EnemyCharacterConfig
		? FMath::Max(0.1f, EnemyCharacterConfig->DeathDespawnDelay)
		: 3.0f;
	GetWorldTimerManager().SetTimer(
		DeathDespawnTimerHandle,
		this,
		&AEnemyCharacter::FinishDeath,
		DespawnDelay,
		false
	);
}

void AEnemyCharacter::GrantKillReward(AActor* Killer)
{
	if (!Killer || !EnemyCharacterConfig) return;
	if (EnemyCharacterConfig->KillUltimateCharge <= 0.0f) return;

	IAbilitySystemInterface* KillerAbilityInterface = Cast<IAbilitySystemInterface>(Killer);
	UAbilitySystemComponent* KillerAbilitySystemComponent = KillerAbilityInterface
		? KillerAbilityInterface->GetAbilitySystemComponent()
		: nullptr;
	if (!KillerAbilitySystemComponent) return;

	FGameplayEffectContextHandle EffectContext = KillerAbilitySystemComponent->MakeEffectContext();
	FGameplayEffectSpecHandle GainSpecHandle = KillerAbilitySystemComponent->MakeOutgoingSpec(
		UGE_GainResource::StaticClass(),
		1.0f,
		EffectContext
	);
	if (!GainSpecHandle.IsValid()) return;

	GainSpecHandle.Data->SetSetByCallerMagnitude(
		RiftGameplayTags::SetByCaller_UltimateCharge,
		EnemyCharacterConfig->KillUltimateCharge
	);
	KillerAbilitySystemComponent->ApplyGameplayEffectSpecToTarget(
		*GainSpecHandle.Data.Get(),
		KillerAbilitySystemComponent
	);
}

void AEnemyCharacter::Multicast_PlayDeath_Implementation(const ERiftHitReactDirection Direction)
{
	bIsDead = true;
	LastHitReactDirection = Direction;

	if (HealthBarWidgetComp)
	{
		HealthBarWidgetComp->SetVisibility(false);
	}

	const UEnemyAnimationConfig* AnimationConfig = EnemyCharacterConfig ? EnemyCharacterConfig->EnemyAnimationConfig : nullptr;
	UAnimMontage* DeathMontage = AnimationConfig ? AnimationConfig->DeathMontage : nullptr;
	if (DeathMontage)
	{
		if (USkeletalMeshComponent* CharacterMesh = GetMesh())
		{
			if (UAnimInstance* AnimInstance = CharacterMesh->GetAnimInstance())
			{
				AnimInstance->Montage_Play(DeathMontage);
				AnimInstance->Montage_JumpToSection(GetHitReactSectionName(Direction), DeathMontage);
			}
		}
	}

	OnDeathVisual(Direction);
}

void AEnemyCharacter::FinishDeath()
{
	Destroy();
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

void AEnemyCharacter::StartEnemyBehavior()
{
	if (!HasAuthority() || bHasStartedEnemyBehavior || bIsDead) return;

	if (ARiftEnemyAIController* RiftAIController = Cast<ARiftEnemyAIController>(GetController()))
	{
		bHasStartedEnemyBehavior = true;
		RiftAIController->StartEnemyBehavior(this);
	}
}

void AEnemyCharacter::StartSpawnIntroOrAI()
{
	if (!HasAuthority() || bIsDead) return;

	const UEnemyAnimationConfig* AnimationConfig = EnemyCharacterConfig ? EnemyCharacterConfig->EnemyAnimationConfig : nullptr;
	UAnimMontage* SpawnIntroMontage = AnimationConfig ? AnimationConfig->SpawnIntroMontage : nullptr;
	if (!SpawnIntroMontage)
	{
		StartEnemyBehavior();
		return;
	}

	SetIntroState(true);
	StopAIMovement();
	Multicast_PlaySpawnIntro();

	const float SpawnIntroDuration = SpawnIntroMontage->GetPlayLength();
	if (SpawnIntroDuration <= 0.0f)
	{
		FinishSpawnIntro();
		return;
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			SpawnIntroTimerHandle,
			this,
			&AEnemyCharacter::FinishSpawnIntro,
			SpawnIntroDuration,
			false
		);
	}
}

void AEnemyCharacter::FinishSpawnIntro()
{
	if (!HasAuthority()) return;

	SetIntroState(false);
	StartEnemyBehavior();
}

void AEnemyCharacter::PlaySpawnIntroMontage()
{
	const UEnemyAnimationConfig* AnimationConfig = EnemyCharacterConfig ? EnemyCharacterConfig->EnemyAnimationConfig : nullptr;
	UAnimMontage* SpawnIntroMontage = AnimationConfig ? AnimationConfig->SpawnIntroMontage : nullptr;
	if (!SpawnIntroMontage) return;

	USkeletalMeshComponent* CharacterMesh = GetMesh();
	if (!CharacterMesh) return;

	UAnimInstance* AnimInstance = CharacterMesh->GetAnimInstance();
	if (!AnimInstance) return;

	AnimInstance->Montage_Play(SpawnIntroMontage);
}

void AEnemyCharacter::FinishDiscoverReaction()
{
	if (!HasAuthority()) return;

	SetDiscoveringState(false);
}

void AEnemyCharacter::PlayDiscoverMontage()
{
	const UEnemyAnimationConfig* AnimationConfig = EnemyCharacterConfig ? EnemyCharacterConfig->EnemyAnimationConfig : nullptr;
	UAnimMontage* DiscoverMontage = AnimationConfig ? AnimationConfig->DiscoverMontage : nullptr;
	if (!DiscoverMontage) return;

	USkeletalMeshComponent* CharacterMesh = GetMesh();
	if (!CharacterMesh) return;

	UAnimInstance* AnimInstance = CharacterMesh->GetAnimInstance();
	if (!AnimInstance) return;

	AnimInstance->Montage_Play(DiscoverMontage);
}

void AEnemyCharacter::StopAIMovement()
{
	if (AController* EnemyController = GetController())
	{
		EnemyController->StopMovement();
	}
}

void AEnemyCharacter::SetStaggeredState(const bool bInStaggered)
{
	if (!AbilitySystemComponent) return;

	const int32 NewCount = bInStaggered ? 1 : 0;
	AbilitySystemComponent->SetLooseGameplayTagCount(RiftGameplayTags::State_Staggered, NewCount);
	AbilitySystemComponent->SetReplicatedLooseGameplayTagCount(RiftGameplayTags::State_Staggered, NewCount);
}

void AEnemyCharacter::SetIntroState(const bool bInIntro)
{
	if (!AbilitySystemComponent) return;
	if (bInIntro && (bIsDead || IsStaggeredForAI())) return;

	const int32 NewCount = bInIntro ? 1 : 0;
	AbilitySystemComponent->SetLooseGameplayTagCount(RiftGameplayTags::State_Intro, NewCount);
	AbilitySystemComponent->SetReplicatedLooseGameplayTagCount(RiftGameplayTags::State_Intro, NewCount);
}

void AEnemyCharacter::SetDiscoveringState(const bool bInDiscovering)
{
	if (!AbilitySystemComponent) return;
	if (bInDiscovering && (bIsDead || IsStaggeredForAI() || IsIntroForAI())) return;

	const int32 NewCount = bInDiscovering ? 1 : 0;
	AbilitySystemComponent->SetLooseGameplayTagCount(RiftGameplayTags::State_Discovering, NewCount);
	AbilitySystemComponent->SetReplicatedLooseGameplayTagCount(RiftGameplayTags::State_Discovering, NewCount);
}

void AEnemyCharacter::SetBlockingState(const bool bInBlocking)
{
	if (!AbilitySystemComponent) return;
	if (bInBlocking && (bIsDead || IsStaggeredForAI())) return;

	const int32 NewCount = bInBlocking ? 1 : 0;
	AbilitySystemComponent->SetLooseGameplayTagCount(RiftGameplayTags::State_Blocking, NewCount);
	AbilitySystemComponent->SetReplicatedLooseGameplayTagCount(RiftGameplayTags::State_Blocking, NewCount);
}

void AEnemyCharacter::EnterStaggered(const float Duration)
{
	if (!HasAuthority() || bIsDead || !AbilitySystemComponent) return;

	SetBlockingState(false);
	SetIntroState(false);
	SetDiscoveringState(false);

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(SpawnIntroTimerHandle);
		World->GetTimerManager().ClearTimer(DiscoverTimerHandle);
	}

	FGameplayTagContainer AttackTags;
	AttackTags.AddTag(RiftGameplayTags::Ability_Enemy_MeleeAttack);
	AbilitySystemComponent->CancelAbilities(&AttackTags);

	SetStaggeredState(true);
	StopAIMovement();

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(StaggeredTimerHandle);
		World->GetTimerManager().SetTimer(
			StaggeredTimerHandle,
			this,
			&AEnemyCharacter::ExitStaggered,
			FMath::Max(0.1f, Duration),
			false
		);
	}
}

void AEnemyCharacter::ExitStaggered()
{
	if (!HasAuthority()) return;

	if (AbilitySystemComponent)
	{
		SetStaggeredState(false);
	}

	CustomTimeDilation = 1.0f;
	StartEnemyBehavior();
}

void AEnemyCharacter::RestorePoise()
{
	if (!HasAuthority() || !AttributeSet) return;

	AttributeSet->SetPoise(AttributeSet->GetMaxPoise());
}
