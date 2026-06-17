// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/PlayerCharacter.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystem/Attributes/RiftPlayerAttributeSet.h"
#include "AbilitySystem/Attributes/RiftResourceAttributeSet.h"
#include "AbilitySystem/Effects/GE_GainResource.h"
#include "AbilitySystem/Effects/GE_StaminaRegen.h"
#include "AbilitySystem/RiftGameplayTags.h"
#include "Abilities/GameplayAbility.h"
#include "Animation/AnimInstance.h"
#include "Appearance/PlayerAppearanceComponent.h"
#include "Combat/RiftCombatFeedbackComponent.h"
#include "Combat/RiftTargetAssistComponent.h"
#include "Combat/RiftWeaponTraceComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Core/RiftGameplayGameMode.h"
#include "Data/Player/PlayerClassConfig.h"
#include "Data/Player/Animation/PlayerAnimationConfig.h"
#include "Data/Player/Combat/PlayerCombatConfig.h"
#include "Debug/Logger.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "Player/BasePlayerState.h"
#include "TimerManager.h"

APlayerCharacter::APlayerCharacter()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = true;
    InitPlayerProperties();
    InitMovementSettings();
    TargetAssistComponent = CreateDefaultSubobject<URiftTargetAssistComponent>(TEXT("TargetAssistComponent"));
    WeaponTraceComponent = CreateDefaultSubobject<URiftWeaponTraceComponent>(TEXT("WeaponTraceComponent"));
    CombatFeedbackComponent = CreateDefaultSubobject<URiftCombatFeedbackComponent>(TEXT("CombatFeedbackComponent"));
    AppearanceComponent = CreateDefaultSubobject<UPlayerAppearanceComponent>(TEXT("AppearanceComponent"));
    InitCameraComponents();

    HairMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("HairMesh"));
    HairMesh->SetupAttachment(GetMesh());

    ArmUpperLeftMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ArmUpperLeftMesh"));
    ArmUpperLeftMesh->SetupAttachment(GetMesh());

    ArmUpperRightMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ArmUpperRightMesh"));
    ArmUpperRightMesh->SetupAttachment(GetMesh());

    AppearanceComponent->SetMasterMeshComponent(GetMesh());
    AppearanceComponent->RegisterAppearanceMeshComponent(ERiftPlayerAppearanceSlot::Hair, HairMesh);
    AppearanceComponent->RegisterAppearanceMeshComponent(ERiftPlayerAppearanceSlot::ArmUpperLeft, ArmUpperLeftMesh);
    AppearanceComponent->RegisterAppearanceMeshComponent(ERiftPlayerAppearanceSlot::ArmUpperRight, ArmUpperRightMesh);
    AppearanceComponent->RefreshLeaderPose();
}

void APlayerCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    ApplyCameraRelativeMovementInput();
    UpdateAssistedFacing(DeltaTime);

    if (FacingMode == ERiftCharacterFacingMode::Movement)
    {
        UpdateMovementRotationRate(DeltaTime);
    }
}

void APlayerCharacter::PostInitializeComponents()
{
    Super::PostInitializeComponents();
    ApplyClassConfigOnAllRoles();
}

void APlayerCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(APlayerCharacter, PlayerClassConfig);
}

UPlayerAnimationConfig* APlayerCharacter::GetPlayerAnimationConfig() const
{
    return PlayerClassConfig ? PlayerClassConfig->PlayerAnimationConfig : nullptr;
}

void APlayerCharacter::SelectPlayerClass_Implementation(UPlayerClassConfig* NewPlayerClassConfig)
{
    if (!NewPlayerClassConfig || PlayerClassConfig == NewPlayerClassConfig) return;

    PlayerClassConfig = NewPlayerClassConfig;
    AssemblePlayerClass();
    ForceNetUpdate();
}

void APlayerCharacter::ApplyClassConfigForPreview(UPlayerClassConfig* PreviewClassConfig)
{
    if (!PreviewClassConfig) return;

    PlayerClassConfig = PreviewClassConfig;
    ApplyClassConfigOnAllRoles();

    if (AppearanceComponent)
    {
        AppearanceComponent->RefreshLeaderPose();
    }
}

void APlayerCharacter::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);

    if (USkeletalMeshComponent* CharacterMesh = GetMesh())
    {
        CharacterMesh->VisibilityBasedAnimTickOption =
            EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
    }

    if (UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponent())
    {
        AbilitySystemComponent->InitAbilityActorInfo(GetPlayerState(), this);
    }

    if (HasAuthority())
    {
        const ABasePlayerState* RiftPlayerState = GetPlayerState<ABasePlayerState>();
        UPlayerClassConfig* SelectedPlayerClassConfig = RiftPlayerState
            ? RiftPlayerState->GetSelectedPlayerClassConfig()
            : nullptr;
        if (SelectedPlayerClassConfig)
        {
            PlayerClassConfig = SelectedPlayerClassConfig;
        }
    }

    AssemblePlayerClass();
    ApplyAppearanceFromPlayerState();
}

void APlayerCharacter::OnRep_PlayerState()
{
    Super::OnRep_PlayerState();

    if (UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponent())
    {
        AbilitySystemComponent->InitAbilityActorInfo(GetPlayerState(), this);
    }

    ApplyAppearanceFromPlayerState();
}

void APlayerCharacter::UnPossessed()
{
    Super::UnPossessed();
    ClearMovementInputCache();
    StopAssistedFacing();
    SetFacingMode(ERiftCharacterFacingMode::Movement);
    CustomTimeDilation = 1.0f;
}

void APlayerCharacter::PawnClientRestart()
{
    Super::PawnClientRestart();
    ClearMovementInputCache();
    StopAssistedFacing();
    SetFacingMode(ERiftCharacterFacingMode::Movement);
}

bool APlayerCharacter::HasMovementInput() const
{
    return !MovementInputVector.IsNearlyZero();
}

bool APlayerCharacter::IsMovementAccelerating() const
{
    const UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
    return MovementComponent && !MovementComponent->GetCurrentAcceleration().IsNearlyZero();
}

UAbilitySystemComponent* APlayerCharacter::GetAbilitySystemComponent() const
{
    const ABasePlayerState* RiftPlayerState = GetPlayerState<ABasePlayerState>();
    return RiftPlayerState ? RiftPlayerState->GetAbilitySystemComponent() : nullptr;
}

void APlayerCharacter::HandleMove(const FVector2D& InputValue)
{
    if (IsDead() || IsInHeavyHitState())
    {
        ClearMovementInputCache();
        return;
    }

    MovementInputVector = InputValue;

    if (InputValue.IsNearlyZero())
    {
        ClearMovementInputCache();
    }
}

void APlayerCharacter::SetFacingMode(const ERiftCharacterFacingMode NewFacingMode)
{
    if (FacingMode == NewFacingMode) return;

    FacingMode = NewFacingMode;
    ApplyFacingModeToMovement();
}

ERiftCharacterFacingMode APlayerCharacter::GetFacingMode() const
{
    return FacingMode;
}

void APlayerCharacter::StartAssistedFacing(const FRotator& TargetRotation, const float Duration, const float RotationSpeed)
{
    if (FacingMode != ERiftCharacterFacingMode::CombatAssist)
    {
        FLogger::Log(
            this,
            TEXT("Warning: StartAssistedFacing ignored because FacingMode is not CombatAssist"),
            ELogSystem::Character,
            ELogOutputType::LogOnly
        );
        return;
    }

    AssistedFacingTargetRotation = FRotator(0.0f, TargetRotation.Yaw, 0.0f);
    AssistedFacingRotationSpeed = RotationSpeed > 0.0f ? RotationSpeed : 1200.0f;

    if (Duration <= 0.0f)
    {
        const FRotator CurrentRotation = GetActorRotation();
        SetActorRotation(FRotator(CurrentRotation.Pitch, AssistedFacingTargetRotation.Yaw, CurrentRotation.Roll));
        StopAssistedFacing();
        return;
    }

    AssistedFacingTimeRemaining = Duration;
    bIsAssistedFacing = true;
}

void APlayerCharacter::StopAssistedFacing()
{
    bIsAssistedFacing = false;
    AssistedFacingTimeRemaining = 0.0f;
}

void APlayerCharacter::ClearMovementInputCache()
{
    MovementInputVector = FVector2D::ZeroVector;
}

FVector APlayerCharacter::GetCameraRelativeMoveDirection() const
{
    if (MovementInputVector.IsNearlyZero()) return FVector::ZeroVector;

    const AController* CurrentController = GetController();
    if (!CurrentController) return GetActorForwardVector();

    const FRotator YawRotation(0.0f, CurrentController->GetControlRotation().Yaw, 0.0f);
    const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
    const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

    return (ForwardDirection * MovementInputVector.Y + RightDirection * MovementInputVector.X).GetSafeNormal();
}

void APlayerCharacter::ActivatePerfectDodgeWindow(const FVector& Origin, const float Duration)
{
    PerfectDodgeOrigin = Origin;
    bPerfectDodgeWindowActive = true;

    UWorld* World = GetWorld();
    if (!World) return;

    World->GetTimerManager().ClearTimer(PerfectDodgeWindowTimerHandle);
    World->GetTimerManager().SetTimer(
        PerfectDodgeWindowTimerHandle,
        this,
        &APlayerCharacter::DeactivatePerfectDodgeWindow,
        Duration,
        false
    );
}

void APlayerCharacter::HandlePerfectDodge(AActor* InstigatorEnemy)
{
    static_cast<void>(InstigatorEnemy);

    FLogger::Log(this, TEXT("Perfect Dodge!"));

    if (!HasAuthority()) return;

    bPerfectDodgeWindowActive = false;
    GetWorldTimerManager().ClearTimer(PerfectDodgeWindowTimerHandle);

    const UPlayerClassConfig* ClassConfig = GetPlayerClassConfig();
    const UPlayerCombatConfig* CombatConfig = ClassConfig ? ClassConfig->PlayerCombatConfig : nullptr;
    UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponent();
    if (CombatConfig && AbilitySystemComponent)
    {
        FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
        FGameplayEffectSpecHandle GainSpec = AbilitySystemComponent->MakeOutgoingSpec(
            UGE_GainResource::StaticClass(),
            1.0f,
            EffectContext
        );
        if (GainSpec.IsValid())
        {
            GainSpec.Data->SetSetByCallerMagnitude(
                RiftGameplayTags::SetByCaller_UltimateCharge,
                CombatConfig->PerfectDodgeUltimateChargeReward
            );
            AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*GainSpec.Data.Get());
        }
    }

}

void APlayerCharacter::HandleDeath()
{
    HandleDeath(nullptr);
}

void APlayerCharacter::HandleDeath(AActor* DeathInstigator)
{
    if (!HasAuthority() || bIsDead) return;

    bIsDead = true;
    LastHitReactDirection = DeathInstigator
        ? CalculateHitReactDirection(this, DeathInstigator->GetActorLocation())
        : ERiftHitReactDirection::Front;

    GetWorldTimerManager().ClearTimer(PerfectDodgeWindowTimerHandle);
    bPerfectDodgeWindowActive = false;
    CustomTimeDilation = 1.0f;
    ClearMovementInputCache();
    StopAssistedFacing();
    ActiveHeavyHitMontage.Reset();

    if (UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponent())
    {
        AbilitySystemComponent->CancelAllAbilities();
        SetHeavyHitState(false);
        AbilitySystemComponent->AddLooseGameplayTag(RiftGameplayTags::State_Dead);
        AbilitySystemComponent->SetUserAbilityActivationInhibited(true);
    }

    if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
    {
        MovementComponent->StopMovementImmediately();
        MovementComponent->DisableMovement();
    }

    SetActorEnableCollision(false);
    Client_SetDeadControlState(true);
    Multicast_PlayDeath(LastHitReactDirection);

    if (UWorld* World = GetWorld())
    {
        if (ARiftGameplayGameMode* GameplayGameMode = World->GetAuthGameMode<ARiftGameplayGameMode>())
        {
            GameplayGameMode->NotifyPlayerDied(this);
        }
    }
}

void APlayerCharacter::Client_DisableInputOnDeath_Implementation()
{
    Client_SetDeadControlState_Implementation(true);
}

void APlayerCharacter::Client_SetDeadControlState_Implementation(const bool bDead)
{
    if (AController* CurrentController = GetController())
    {
        CurrentController->SetIgnoreMoveInput(bDead);
        CurrentController->SetIgnoreLookInput(bDead);
    }

    if (bDead)
    {
        ClearMovementInputCache();
    }

    if (UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponent())
    {
        AbilitySystemComponent->SetUserAbilityActivationInhibited(bDead);
    }
}

void APlayerCharacter::Client_SetHeavyHitControlState_Implementation(const bool bInHeavyHit)
{
    ClearMovementInputCache();

    if (UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponent())
    {
        AbilitySystemComponent->SetUserAbilityActivationInhibited(bInHeavyHit);
    }
}

void APlayerCharacter::Multicast_PlayDeath_Implementation(const ERiftHitReactDirection Direction)
{
    bIsDead = true;
    LastHitReactDirection = Direction;

    const UPlayerAnimationConfig* AnimationConfig = GetPlayerAnimationConfig();
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

void APlayerCharacter::ReviveAtTransform(const FTransform& ReviveTransform)
{
    if (!HasAuthority())
    {
        return;
    }

    bIsDead = false;
    LastHitReactDirection = ERiftHitReactDirection::Front;
    GetWorldTimerManager().ClearTimer(PerfectDodgeWindowTimerHandle);
    bPerfectDodgeWindowActive = false;
    PerfectDodgeOrigin = FVector::ZeroVector;
    ActiveHeavyHitMontage.Reset();
    CustomTimeDilation = 1.0f;
    ClearMovementInputCache();
    StopAssistedFacing();
    SetFacingMode(ERiftCharacterFacingMode::Movement);

    SetActorTransform(ReviveTransform, false, nullptr, ETeleportType::TeleportPhysics);
    SetActorEnableCollision(true);

    if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
    {
        MovementComponent->StopMovementImmediately();
        MovementComponent->SetMovementMode(MOVE_Walking);
    }

    UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponent();
    if (AbilitySystemComponent)
    {
        AbilitySystemComponent->SetLooseGameplayTagCount(RiftGameplayTags::State_Dead, 0);
        AbilitySystemComponent->SetReplicatedLooseGameplayTagCount(RiftGameplayTags::State_Dead, 0);
        SetHeavyHitState(false);

        const float MaxHealth = AbilitySystemComponent->GetNumericAttribute(
            URiftPlayerAttributeSet::GetMaxHealthAttribute()
        );
        const float MaxPoise = AbilitySystemComponent->GetNumericAttribute(
            URiftPlayerAttributeSet::GetMaxPoiseAttribute()
        );
        const float MaxStamina = AbilitySystemComponent->GetNumericAttribute(
            URiftPlayerAttributeSet::GetMaxStaminaAttribute()
        );

        AbilitySystemComponent->SetNumericAttributeBase(
            URiftPlayerAttributeSet::GetHealthAttribute(),
            MaxHealth
        );
        AbilitySystemComponent->SetNumericAttributeBase(
            URiftPlayerAttributeSet::GetPoiseAttribute(),
            MaxPoise
        );
        AbilitySystemComponent->SetNumericAttributeBase(
            URiftPlayerAttributeSet::GetStaminaAttribute(),
            MaxStamina
        );
        AbilitySystemComponent->SetUserAbilityActivationInhibited(false);
    }
    else
    {
        SetHeavyHitState(false);
    }

    Client_SetDeadControlState(false);
    Multicast_PlayReviveVisual();
}

void APlayerCharacter::Multicast_PlayReviveVisual_Implementation()
{
    bIsDead = false;
    OnPlayerRevived();
}

void APlayerCharacter::HandleHitFeedback(
    const ERiftPlayerHitFeedbackPolicy FeedbackPolicy,
    AActor* DamageInstigator,
    const float DamageValue
)
{
    if (!HasAuthority() || IsDead()) return;

    UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponent();
    if (AbilitySystemComponent &&
        (AbilitySystemComponent->HasMatchingGameplayTag(RiftGameplayTags::State_SuperArmor_Red) ||
            AbilitySystemComponent->HasMatchingGameplayTag(RiftGameplayTags::State_Hit_Heavy)))
    {
        return;
    }

    const ERiftHitReactDirection Direction = DamageInstigator
        ? CalculateHitReactDirection(this, DamageInstigator->GetActorLocation())
        : ERiftHitReactDirection::Front;

    switch (FeedbackPolicy)
    {
    case ERiftPlayerHitFeedbackPolicy::None:
        break;
    case ERiftPlayerHitFeedbackPolicy::FeedbackOnly:
        Multicast_PlayHitFeedback(Direction, DamageValue, DamageInstigator);
        break;
    case ERiftPlayerHitFeedbackPolicy::LightHit:
        Multicast_PlayLightHit(Direction, DamageValue, DamageInstigator);
        break;
    default:
        break;
    }
}

void APlayerCharacter::HandlePoiseBroken(AActor* DamageInstigator)
{
    if (!HasAuthority() || IsDead()) return;

    UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponent();
    if (AbilitySystemComponent &&
        AbilitySystemComponent->HasMatchingGameplayTag(RiftGameplayTags::State_SuperArmor_Red))
    {
        return;
    }

    HandleHeavyHit(DamageInstigator);
}

void APlayerCharacter::HandleHeavyHit(AActor* DamageInstigator)
{
    if (!HasAuthority() || IsDead()) return;

    UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponent();
    if (!AbilitySystemComponent) return;

    if (AbilitySystemComponent->HasMatchingGameplayTag(RiftGameplayTags::State_Hit_Heavy) ||
        AbilitySystemComponent->HasMatchingGameplayTag(RiftGameplayTags::State_SuperArmor_Red))
    {
        return;
    }

    const UPlayerAnimationConfig* AnimationConfig = GetPlayerAnimationConfig();
    UAnimMontage* HeavyHitMontage = AnimationConfig ? AnimationConfig->HeavyHitMontage : nullptr;
    if (!HeavyHitMontage) return;

    LastHitReactDirection = DamageInstigator
        ? CalculateHitReactDirection(this, DamageInstigator->GetActorLocation())
        : ERiftHitReactDirection::Front;

    AbilitySystemComponent->CancelAllAbilities();
    AbilitySystemComponent->SetUserAbilityActivationInhibited(true);
    ActiveHeavyHitMontage = HeavyHitMontage;
    SetHeavyHitState(true);

    ClearMovementInputCache();
    StopAssistedFacing();
    if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
    {
        MovementComponent->StopMovementImmediately();
    }

    Client_SetHeavyHitControlState(true);
    Multicast_PlayHeavyHit(LastHitReactDirection);
}

void APlayerCharacter::FinishHeavyHit()
{
    if (!HasAuthority())
    {
        if (IsLocallyControlled())
        {
            Server_FinishHeavyHit();
        }
        return;
    }

    if (IsDead()) return;

    UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponent();
    if (!AbilitySystemComponent) return;
    if (!AbilitySystemComponent->HasMatchingGameplayTag(RiftGameplayTags::State_Hit_Heavy)) return;

    SetHeavyHitState(false);
    ActiveHeavyHitMontage.Reset();
    AbilitySystemComponent->SetUserAbilityActivationInhibited(false);
    Client_SetHeavyHitControlState(false);

    if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
    {
        if (MovementComponent->MovementMode == MOVE_None)
        {
            MovementComponent->SetMovementMode(MOVE_Walking);
        }
    }
}

void APlayerCharacter::Server_FinishHeavyHit_Implementation()
{
    FinishHeavyHit();
}

void APlayerCharacter::OnHeavyHitMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    static_cast<void>(bInterrupted);

    if (!HasAuthority()) return;
    if (IsDead()) return;
    if (!Montage || ActiveHeavyHitMontage.Get() != Montage) return;

    UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponent();
    if (!AbilitySystemComponent) return;
    if (!AbilitySystemComponent->HasMatchingGameplayTag(RiftGameplayTags::State_Hit_Heavy)) return;

    FinishHeavyHit();
}

bool APlayerCharacter::IsInHeavyHitState() const
{
    const UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponent();
    return AbilitySystemComponent &&
        AbilitySystemComponent->HasMatchingGameplayTag(RiftGameplayTags::State_Hit_Heavy);
}

void APlayerCharacter::Multicast_PlayHitFeedback_Implementation(
    const ERiftHitReactDirection Direction,
    const float DamageValue,
    AActor* DamageInstigator
)
{
    LastHitReactDirection = Direction;
    OnPlayerHitDamaged(Direction, DamageValue, DamageInstigator);
}

void APlayerCharacter::Multicast_PlayLightHit_Implementation(
    const ERiftHitReactDirection Direction,
    const float DamageValue,
    AActor* DamageInstigator
)
{
    LastHitReactDirection = Direction;
    OnPlayerHitDamaged(Direction, DamageValue, DamageInstigator);

    const UPlayerAnimationConfig* AnimationConfig = GetPlayerAnimationConfig();
    UAnimMontage* LightHitMontage = AnimationConfig ? AnimationConfig->LightHitMontage : nullptr;
    if (!LightHitMontage) return;

    USkeletalMeshComponent* CharacterMesh = GetMesh();
    if (!CharacterMesh) return;

    UAnimInstance* AnimInstance = CharacterMesh->GetAnimInstance();
    if (!AnimInstance) return;

    AnimInstance->Montage_Play(LightHitMontage);
    AnimInstance->Montage_JumpToSection(GetHitReactSectionName(Direction), LightHitMontage);
}

void APlayerCharacter::Multicast_PlayHeavyHit_Implementation(const ERiftHitReactDirection Direction)
{
    LastHitReactDirection = Direction;

    const UPlayerAnimationConfig* AnimationConfig = GetPlayerAnimationConfig();
    UAnimMontage* HeavyHitMontage = AnimationConfig ? AnimationConfig->HeavyHitMontage : nullptr;
    if (!HeavyHitMontage)
    {
        if (HasAuthority())
        {
            FinishHeavyHit();
        }
        return;
    }

    USkeletalMeshComponent* CharacterMesh = GetMesh();
    if (!CharacterMesh)
    {
        if (HasAuthority())
        {
            FinishHeavyHit();
        }
        return;
    }

    UAnimInstance* AnimInstance = CharacterMesh->GetAnimInstance();
    if (!AnimInstance)
    {
        if (HasAuthority())
        {
            FinishHeavyHit();
        }
        return;
    }

    const float MontageLength = AnimInstance->Montage_Play(HeavyHitMontage);
    if (MontageLength <= 0.0f)
    {
        if (HasAuthority())
        {
            FinishHeavyHit();
        }
        return;
    }

    AnimInstance->Montage_JumpToSection(GetHitReactSectionName(Direction), HeavyHitMontage);

    if (HasAuthority())
    {
        ActiveHeavyHitMontage = HeavyHitMontage;
        FOnMontageEnded EndDelegate;
        EndDelegate.BindUObject(this, &APlayerCharacter::OnHeavyHitMontageEnded);
        AnimInstance->Montage_SetEndDelegate(EndDelegate, HeavyHitMontage);
    }
}

void APlayerCharacter::InitPlayerProperties()
{
    // Network
    bReplicates = true;
    SetReplicateMovement(true);
}

void APlayerCharacter::InitCameraComponents()
{
    SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComponent"));
    SpringArmComponent->SetupAttachment(RootComponent);
    SpringArmComponent->bUsePawnControlRotation = true;
    SpringArmComponent->bEnableCameraLag = true;
    SpringArmComponent->CameraLagSpeed = 10.0f;
    SpringArmComponent->CameraLagMaxDistance = 150.0f;

    CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
    CameraComponent->SetupAttachment(SpringArmComponent);
    CameraComponent->bUsePawnControlRotation = false;
}

void APlayerCharacter::AssemblePlayerClass()
{
    ApplyClassConfigOnAllRoles();

    if (!HasAuthority()) return;

    ApplyClassConfigOnAuthority();
}

void APlayerCharacter::ApplyAppearanceFromPlayerState()
{
    const ABasePlayerState* RiftPlayerState = GetPlayerState<ABasePlayerState>();
    if (!AppearanceComponent || !RiftPlayerState)
    {
        return;
    }

    AppearanceComponent->ApplyAppearanceSelection(RiftPlayerState->GetConfirmedAppearanceSelection());
}

void APlayerCharacter::OnRep_PlayerClassConfig()
{
    ApplyClassConfigOnAllRoles();
}

void APlayerCharacter::ApplyClassConfigOnAllRoles()
{
    ApplyAnimationConfig();
    ApplyMovementSettings();
    ApplyWeaponsFromConfig();
}

void APlayerCharacter::ApplyClassConfigOnAuthority()
{
    if (!HasAuthority()) return;

    ApplyCommonAttributesFromConfig();
    GrantAbilitiesFromClassConfig();
}

void APlayerCharacter::ApplyAnimationConfig() const
{
    const UPlayerAnimationConfig* AnimationConfig = GetPlayerAnimationConfig();
    if (!AnimationConfig) return;

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

void APlayerCharacter::ApplyCommonAttributesFromConfig()
{
    if (!HasAuthority()) return;

    if (!PlayerClassConfig) return;

    UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponent();
    if (!AbilitySystemComponent) return;

    AbilitySystemComponent->SetNumericAttributeBase(
        URiftPlayerAttributeSet::GetMaxHealthAttribute(),
        PlayerClassConfig->MaxHealth
    );
    AbilitySystemComponent->SetNumericAttributeBase(
        URiftPlayerAttributeSet::GetHealthAttribute(),
        PlayerClassConfig->Health
    );
    AbilitySystemComponent->SetNumericAttributeBase(
        URiftPlayerAttributeSet::GetMaxStaminaAttribute(),
        PlayerClassConfig->MaxStamina
    );
    AbilitySystemComponent->SetNumericAttributeBase(
        URiftPlayerAttributeSet::GetStaminaAttribute(),
        PlayerClassConfig->Stamina
    );
    AbilitySystemComponent->SetNumericAttributeBase(
        URiftPlayerAttributeSet::GetMaxPoiseAttribute(),
        PlayerClassConfig->MaxPoise
    );
    AbilitySystemComponent->SetNumericAttributeBase(
        URiftPlayerAttributeSet::GetPoiseAttribute(),
        PlayerClassConfig->Poise
    );
    AbilitySystemComponent->SetNumericAttributeBase(
        URiftResourceAttributeSet::GetMaxUltimateChargeAttribute(),
        PlayerClassConfig->MaxUltimateCharge
    );
    AbilitySystemComponent->SetNumericAttributeBase(
        URiftResourceAttributeSet::GetUltimateChargeAttribute(),
        PlayerClassConfig->UltimateCharge
    );

    if (StaminaRegenEffectHandle.IsValid())
    {
        AbilitySystemComponent->RemoveActiveGameplayEffect(StaminaRegenEffectHandle);
        StaminaRegenEffectHandle = FActiveGameplayEffectHandle();
    }

    const float RegenPerTick = PlayerClassConfig->StaminaRegenRate * 0.1f;
    FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
    EffectContext.AddSourceObject(this);

    FGameplayEffectSpecHandle StaminaRegenSpec = AbilitySystemComponent->MakeOutgoingSpec(
        UGE_StaminaRegen::StaticClass(),
        1.0f,
        EffectContext
    );
    if (!StaminaRegenSpec.IsValid()) return;

    StaminaRegenSpec.Data->SetSetByCallerMagnitude(
        RiftGameplayTags::SetByCaller_StaminaRegen,
        RegenPerTick
    );
    StaminaRegenEffectHandle = AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*StaminaRegenSpec.Data.Get());
}

void APlayerCharacter::ApplyWeaponsFromConfig()
{
    if (!PlayerClassConfig) return;
    ApplyWeapons(PlayerClassConfig->Weapons);
}

void APlayerCharacter::ClearGrantedAbilities()
{
    if (!HasAuthority()) return;

    UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponent();
    if (!AbilitySystemComponent)
    {
        GrantedAbilityHandles.Empty();
        return;
    }

    for (const FGameplayAbilitySpecHandle& AbilityHandle : GrantedAbilityHandles)
    {
        if (AbilityHandle.IsValid())
        {
            AbilitySystemComponent->ClearAbility(AbilityHandle);
        }
    }

    GrantedAbilityHandles.Empty();
}

void APlayerCharacter::DeactivatePerfectDodgeWindow()
{
    bPerfectDodgeWindowActive = false;
}

void APlayerCharacter::SetHeavyHitState(const bool bInHeavyHit)
{
    UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponent();
    if (!AbilitySystemComponent) return;

    const int32 NewCount = bInHeavyHit ? 1 : 0;
    AbilitySystemComponent->SetLooseGameplayTagCount(RiftGameplayTags::State_Hit_Heavy, NewCount);
    AbilitySystemComponent->SetReplicatedLooseGameplayTagCount(RiftGameplayTags::State_Hit_Heavy, NewCount);
}

void APlayerCharacter::GrantAbilitiesFromClassConfig()
{
    if (!HasAuthority()) return;

    ClearGrantedAbilities();

    FLogger::Log(
        this,
        FString::Printf(TEXT("GrantAbilitiesFromClassConfig: PlayerClassConfig=%s"), *GetNameSafe(PlayerClassConfig)),
        ELogSystem::Ability,
        ELogOutputType::LogOnly
    );

    if (!PlayerClassConfig) return;

    FLogger::Log(
        this,
        FString::Printf(TEXT("GrantAbilitiesFromClassConfig: GrantedAbilities.Num=%d"), PlayerClassConfig->GrantedAbilities.Num()),
        ELogSystem::Ability,
        ELogOutputType::LogOnly
    );

    UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponent();
    if (!AbilitySystemComponent) return;

    for (const TSubclassOf<UGameplayAbility>& AbilityClass : PlayerClassConfig->GrantedAbilities)
    {
        FLogger::Log(
            this,
            FString::Printf(TEXT("GrantAbilitiesFromClassConfig: AbilityClass=%s"), *GetNameSafe(AbilityClass.Get())),
            ELogSystem::Ability,
            ELogOutputType::LogOnly
        );

        if (!AbilityClass) continue;

        const FGameplayAbilitySpec AbilitySpec(AbilityClass, 1, INDEX_NONE, this);
        const FGameplayAbilitySpecHandle AbilityHandle = AbilitySystemComponent->GiveAbility(AbilitySpec);
        GrantedAbilityHandles.Add(AbilityHandle);

        FLogger::Log(
            this,
            FString::Printf(
                TEXT("GrantAbilitiesFromClassConfig: GiveAbility HandleValid=%s ActivatableAbilities.Num=%d"),
                AbilityHandle.IsValid() ? TEXT("true") : TEXT("false"),
                AbilitySystemComponent->GetActivatableAbilities().Num()
            ),
            ELogSystem::Ability,
            ELogOutputType::LogOnly
        );
    }
}

void APlayerCharacter::InitMovementSettings()
{
    UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
    if (!MovementComponent) return;

    bUseControllerRotationPitch = false;
    bUseControllerRotationRoll = false;

    ApplyFacingModeToMovement();
}

void APlayerCharacter::ApplyCameraRelativeMovementInput()
{
    if (IsDead() || IsInHeavyHitState()) return;
    if (!Controller || MovementInputVector.IsNearlyZero()) return;

    const FVector2D InputVector = MovementInputVector.GetClampedToMaxSize(1.0f);

    const FRotator ControlRotation = Controller->GetControlRotation();

    // Rotate around Z axis
    const FRotator YawRotation(0.0f, ControlRotation.Yaw, 0.0f);

    const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
    const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

    const FVector MoveVector = ForwardDirection * InputVector.Y + RightDirection * InputVector.X;

    const FVector MoveDirection = MoveVector.GetSafeNormal();
    if (MoveDirection.IsNearlyZero()) return;
    AddMovementInput(MoveDirection, MoveVector.Size());
}

void APlayerCharacter::ApplyFacingModeToMovement()
{
    UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
    if (!MovementComponent) return;

    bUseControllerRotationYaw = false;
    MovementComponent->bUseControllerDesiredRotation = false;

    switch (FacingMode)
    {
    case ERiftCharacterFacingMode::CombatAssist:
        MovementComponent->bOrientRotationToMovement = false;
        break;
    case ERiftCharacterFacingMode::Movement:
    default:
        MovementComponent->bOrientRotationToMovement = true;
        break;
    }
}

void APlayerCharacter::ApplyMovementSettings() const
{
    UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
    if (!MovementComponent) return;

    if (!PlayerClassConfig) return;
    MovementComponent->MaxWalkSpeed = PlayerClassConfig->RunSpeed;

    const float InitialTurnRate = (PlayerClassConfig->MinTurnRate + PlayerClassConfig->MaxTurnRate) / 2.0f;
    MovementComponent->RotationRate = FRotator(0.0f, InitialTurnRate, 0.0f);

    MovementComponent->MaxAcceleration = PlayerClassConfig->MaxAcceleration;
    MovementComponent->BrakingDecelerationWalking = PlayerClassConfig->BrakingDecelerationWalking;
    MovementComponent->bUseSeparateBrakingFriction = true;
    MovementComponent->BrakingFriction = PlayerClassConfig->BrakingFriction;
    MovementComponent->BrakingFrictionFactor = PlayerClassConfig->BrakingFrictionFactor;
    MovementComponent->GroundFriction = PlayerClassConfig->GroundFriction;
}

void APlayerCharacter::UpdateMovementRotationRate(const float DeltaTime)
{
    if (FacingMode != ERiftCharacterFacingMode::Movement) return;

    UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
    if (!MovementComponent) return;

    const FVector Acceleration = MovementComponent->GetCurrentAcceleration().GetSafeNormal2D();
    if (Acceleration.IsNearlyZero()) return;

    const float CurrentYaw = GetActorRotation().Yaw;
    const float DesiredYaw = Acceleration.Rotation().Yaw;

    const float AngleDelta = FMath::Abs(
        FMath::FindDeltaAngleDegrees(CurrentYaw, DesiredYaw)
    );

    const float ConfigMinTurnRate = PlayerClassConfig ? PlayerClassConfig->MinTurnRate : 300.0f;
    const float ConfigMaxTurnRate = PlayerClassConfig ? PlayerClassConfig->MaxTurnRate : 1440.0f;
    const float ConfigMinTurnRateInterpSpeed = PlayerClassConfig ? PlayerClassConfig->MinTurnRateInterpSpeed : 3.0f;
    const float ConfigMaxTurnRateInterpSpeed = PlayerClassConfig ? PlayerClassConfig->MaxTurnRateInterpSpeed : 18.0f;

    const float TargetTurnRate = FMath::GetMappedRangeValueClamped(
        FVector2D(0.0f, 90.0f),
        FVector2D(ConfigMinTurnRate, ConfigMaxTurnRate),
        AngleDelta
    );

    const float DynamicInterpSpeed = FMath::GetMappedRangeValueClamped(
        FVector2D(0.0f, 90.0f),
        FVector2D(ConfigMinTurnRateInterpSpeed, ConfigMaxTurnRateInterpSpeed),
        AngleDelta
    );

    CurrentTurnRate = FMath::FInterpTo(
        CurrentTurnRate,
        TargetTurnRate,
        DeltaTime,
        DynamicInterpSpeed
    );

    MovementComponent->RotationRate = FRotator(0.0f, CurrentTurnRate, 0.0f);
}

void APlayerCharacter::UpdateAssistedFacing(const float DeltaTime)
{
    if (FacingMode != ERiftCharacterFacingMode::CombatAssist) return;
    if (!bIsAssistedFacing) return;

    const FRotator CurrentRotation = GetActorRotation();
    const float NewYaw = FMath::FixedTurn(
        CurrentRotation.Yaw,
        AssistedFacingTargetRotation.Yaw,
        AssistedFacingRotationSpeed * DeltaTime
    );

    SetActorRotation(FRotator(CurrentRotation.Pitch, NewYaw, CurrentRotation.Roll));

    AssistedFacingTimeRemaining -= DeltaTime;
    const float YawDelta = FMath::Abs(
        FMath::FindDeltaAngleDegrees(NewYaw, AssistedFacingTargetRotation.Yaw)
    );
    if (AssistedFacingTimeRemaining <= 0.0f ||
        YawDelta < 0.1f)
    {
        StopAssistedFacing();
    }
}
