// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/PlayerCharacter.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystem/Abilities/GA_TwinSwordRapidSlash.h"
#include "AbilitySystem/Attributes/RiftPlayerAttributeSet.h"
#include "AbilitySystem/Attributes/RiftResourceAttributeSet.h"
#include "AbilitySystem/Effects/GE_GainResource.h"
#include "AbilitySystem/RiftGameplayTags.h"
#include "Abilities/GameplayAbility.h"
#include "Animation/AnimInstance.h"
#include "Appearance/PlayerAppearanceComponent.h"
#include "Combat/RiftCombatFeedbackComponent.h"
#include "Combat/RiftTargetAssistComponent.h"
#include "Combat/RiftWeaponTraceComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Core/RiftGameplayGameMode.h"
#include "Data/Ability/RiftAbilityConfig.h"
#include "Data/Player/PlayerClassConfig.h"
#include "Debug/Logger.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Player/BasePlayerController.h"
#include "Player/BasePlayerState.h"
#include "TimerManager.h"

static constexpr float RiftPlayerDefaultRunSpeed = 600.0f;
static constexpr float RiftPlayerDefaultMaxAcceleration = 900.0f;
static constexpr float RiftPlayerDefaultBrakingDecelerationWalking = 700.0f;
static constexpr float RiftPlayerDefaultBrakingFriction = 3.0f;
static constexpr float RiftPlayerDefaultBrakingFrictionFactor = 1.0f;
static constexpr float RiftPlayerDefaultGroundFriction = 8.0f;
static constexpr float RiftPlayerDefaultMinTurnRate = 200.0f;
static constexpr float RiftPlayerDefaultMaxTurnRate = 1000.0f;
static constexpr float RiftPlayerDefaultMinTurnRateInterpSpeed = 3.0f;
static constexpr float RiftPlayerDefaultMaxTurnRateInterpSpeed = 25.0f;

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

void APlayerCharacter::StartRapidSlashAuraVisual(
    UNiagaraSystem* AuraNiagara,
    const FName AttachSocketName,
    const FVector& LocationOffset,
    const FRotator& RotationOffset,
    const FVector& Scale
)
{
    StopRapidSlashAuraVisual();

    if (!AuraNiagara) return;

    USkeletalMeshComponent* CharacterMesh = GetMesh();
    if (!CharacterMesh) return;

    ActiveRapidSlashAuraComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
        AuraNiagara,
        CharacterMesh,
        AttachSocketName,
        LocationOffset,
        RotationOffset,
        Scale,
        EAttachLocation::KeepRelativeOffset,
        false,
        ENCPoolMethod::None,
        true
    );
}

void APlayerCharacter::StopRapidSlashAuraVisual()
{
    if (!ActiveRapidSlashAuraComponent) return;

    ActiveRapidSlashAuraComponent->Deactivate();
    ActiveRapidSlashAuraComponent->DestroyComponent();
    ActiveRapidSlashAuraComponent = nullptr;
}

void APlayerCharacter::Multicast_StartRapidSlashAuraVisual_Implementation(
    UNiagaraSystem* AuraNiagara,
    const FName AttachSocketName,
    const FVector LocationOffset,
    const FRotator RotationOffset,
    const FVector Scale
)
{
    StartRapidSlashAuraVisual(AuraNiagara, AttachSocketName, LocationOffset, RotationOffset, Scale);
}

void APlayerCharacter::Multicast_StopRapidSlashAuraVisual_Implementation()
{
    StopRapidSlashAuraVisual();
}

void APlayerCharacter::RequestRapidSlashFinisher()
{
    UGA_TwinSwordRapidSlash* RapidSlashAbility = ActiveTwinSwordRapidSlashAbility.Get();
    if (!RapidSlashAbility)
    {
        RapidSlashAbility = UGA_TwinSwordRapidSlash::FindActiveRapidSlashInstance(this);
    }

    if (RapidSlashAbility)
    {
        RapidSlashAbility->RequestFinisherFromInput();
    }

    if (!HasAuthority())
    {
        Server_RequestRapidSlashFinisher();
    }
}

void APlayerCharacter::Server_RequestRapidSlashFinisher_Implementation()
{
    UGA_TwinSwordRapidSlash* RapidSlashAbility = ActiveTwinSwordRapidSlashAbility.Get();
    if (!RapidSlashAbility)
    {
        RapidSlashAbility = UGA_TwinSwordRapidSlash::FindActiveRapidSlashInstance(this);
    }

    if (RapidSlashAbility)
    {
        RapidSlashAbility->RequestFinisherFromInput();
    }
}

void APlayerCharacter::SetActionCancelableState(const bool bCancelable)
{
    UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponent();
    if (!AbilitySystemComponent) return;

    const int32 NewCount = bCancelable ? 1 : 0;
    AbilitySystemComponent->SetLooseGameplayTagCount(RiftGameplayTags::State_Action_Cancelable, NewCount);
    if (AbilitySystemComponent->IsOwnerActorAuthoritative())
    {
        AbilitySystemComponent->SetReplicatedLooseGameplayTagCount(
            RiftGameplayTags::State_Action_Cancelable,
            NewCount
        );
    }
}

void APlayerCharacter::ClearActionCancelableState()
{
    SetActionCancelableState(false);
}

bool APlayerCharacter::IsActionCancelable() const
{
    const UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponent();
    return AbilitySystemComponent &&
        AbilitySystemComponent->HasMatchingGameplayTag(RiftGameplayTags::State_Action_Cancelable);
}

bool APlayerCharacter::IsDodgingForActionCancel() const
{
    const UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponent();
    return AbilitySystemComponent &&
        AbilitySystemComponent->HasMatchingGameplayTag(RiftGameplayTags::State_Dodging);
}

void APlayerCharacter::CancelPlayerActionAbilities(const bool bIncludeGuard, const bool bIncludeDodge)
{
    UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponent();
    if (!AbilitySystemComponent) return;

    FGameplayTagContainer AbilityTags;
    AbilityTags.AddTag(RiftGameplayTags::Ability_Attack_Combo);
    AbilityTags.AddTag(RiftGameplayTags::Ability_Attack_TwinSwordCombo);
    AbilityTags.AddTag(RiftGameplayTags::Ability_Attack_TwinSwordRapidSlash);
    AbilityTags.AddTag(RiftGameplayTags::Ability_Skill_TwinSword_SwordWave);
    AbilityTags.AddTag(RiftGameplayTags::Ability_Skill_TwinSword_Q);

    if (bIncludeGuard)
    {
        AbilityTags.AddTag(RiftGameplayTags::Ability_Guard);
    }

    if (bIncludeDodge)
    {
        AbilityTags.AddTag(RiftGameplayTags::Ability_Dodge);
    }

    AbilitySystemComponent->CancelAbilities(&AbilityTags);
}

void APlayerCharacter::SetActiveTwinSwordRapidSlashAbility(UGA_TwinSwordRapidSlash* Ability)
{
    if (!Ability) return;

    ActiveTwinSwordRapidSlashAbility = Ability;
}

void APlayerCharacter::ClearActiveTwinSwordRapidSlashAbility(UGA_TwinSwordRapidSlash* Ability)
{
    if (ActiveTwinSwordRapidSlashAbility.Get() == Ability)
    {
        ActiveTwinSwordRapidSlashAbility.Reset();
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
        ABasePlayerState* RiftPlayerState = GetPlayerState<ABasePlayerState>();
        UPlayerClassConfig* SelectedPlayerClassConfig = RiftPlayerState
            ? RiftPlayerState->GetSelectedPlayerClassConfig()
            : nullptr;

        if (!SelectedPlayerClassConfig)
        {
            if (ABasePlayerController* RiftPlayerController = Cast<ABasePlayerController>(NewController))
            {
                RiftPlayerController->ApplyCachedLobbySelectionToPlayerState();
                RiftPlayerState = GetPlayerState<ABasePlayerState>();
                SelectedPlayerClassConfig = RiftPlayerState
                    ? RiftPlayerState->GetSelectedPlayerClassConfig()
                    : nullptr;
            }
        }

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
    if (IsDead() ||
        IsInLightHitState() ||
        IsInHeavyHitState())
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

void APlayerCharacter::ActivatePerfectDodgeWindow(
    const FVector& Origin,
    const float Duration,
    const float UltimateChargeReward
)
{
    PerfectDodgeOrigin = Origin;
    PerfectDodgeUltimateChargeReward = FMath::Max(0.0f, UltimateChargeReward);
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

    UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponent();
    if (PerfectDodgeUltimateChargeReward > 0.0f && AbilitySystemComponent)
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
                PerfectDodgeUltimateChargeReward
            );
            AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*GainSpec.Data.Get());
        }
    }
    PerfectDodgeUltimateChargeReward = 0.0f;

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
    PerfectDodgeUltimateChargeReward = 0.0f;
    CustomTimeDilation = 1.0f;
    ClearMovementInputCache();
    StopAssistedFacing();
    ClearActionCancelableState();
    ActiveLightHitMontage.Reset();
    ActiveHeavyHitMontage.Reset();
    Multicast_StopRapidSlashAuraVisual();

    if (UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponent())
    {
        AbilitySystemComponent->CancelAllAbilities();
        SetLightHitState(false);
        SetHeavyHitState(false);
        AbilitySystemComponent->SetLooseGameplayTagCount(RiftGameplayTags::State_Blocking, 0);
        AbilitySystemComponent->SetReplicatedLooseGameplayTagCount(RiftGameplayTags::State_Blocking, 0);
        AbilitySystemComponent->SetLooseGameplayTagCount(RiftGameplayTags::State_Dead, 1);
        AbilitySystemComponent->SetReplicatedLooseGameplayTagCount(RiftGameplayTags::State_Dead, 1);
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

void APlayerCharacter::HandleAttributeDeath(AActor* DeathInstigator)
{
    HandleDeath(DeathInstigator);
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

void APlayerCharacter::Client_ShowDamageIndicator_Implementation(const float DamageValue)
{
    OnDamageIndicator(DamageValue);
}

void APlayerCharacter::Client_SetLightHitControlState_Implementation(const bool bInLightHit)
{
    ClearMovementInputCache();

    if (UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponent())
    {
        AbilitySystemComponent->SetUserAbilityActivationInhibited(bInLightHit);
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
    ClearActionCancelableState();

    UAnimMontage* MontageToPlay = PlayerClassConfig ? PlayerClassConfig->DeathMontage : nullptr;
    if (MontageToPlay)
    {
        if (USkeletalMeshComponent* CharacterMesh = GetMesh())
        {
            if (UAnimInstance* AnimInstance = CharacterMesh->GetAnimInstance())
            {
                AnimInstance->Montage_Play(MontageToPlay);
                AnimInstance->Montage_JumpToSection(GetHitReactSectionName(Direction), MontageToPlay);
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
    PerfectDodgeUltimateChargeReward = 0.0f;
    ActiveLightHitMontage.Reset();
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
        SetLightHitState(false);
        SetHeavyHitState(false);
        AbilitySystemComponent->SetLooseGameplayTagCount(RiftGameplayTags::State_Blocking, 0);
        AbilitySystemComponent->SetReplicatedLooseGameplayTagCount(RiftGameplayTags::State_Blocking, 0);

        const float CurrentMaxHealth = AbilitySystemComponent->GetNumericAttribute(
            URiftPlayerAttributeSet::GetMaxHealthAttribute()
        );

        AbilitySystemComponent->SetNumericAttributeBase(
            URiftPlayerAttributeSet::GetHealthAttribute(),
            CurrentMaxHealth
        );
        AbilitySystemComponent->SetUserAbilityActivationInhibited(false);
    }
    else
    {
        SetLightHitState(false);
        SetHeavyHitState(false);
    }

    Client_SetDeadControlState(false);
    Multicast_PlayReviveVisual();
}

void APlayerCharacter::Multicast_PlayReviveVisual_Implementation()
{
    bIsDead = false;

    if (UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponent())
    {
        AbilitySystemComponent->SetLooseGameplayTagCount(RiftGameplayTags::State_Dead, 0);
        AbilitySystemComponent->SetReplicatedLooseGameplayTagCount(RiftGameplayTags::State_Dead, 0);
        AbilitySystemComponent->SetUserAbilityActivationInhibited(false);
    }

    OnPlayerRevived();
}

void APlayerCharacter::HandlePlayerHitReaction(
    const ERiftPlayerHitReaction Reaction,
    AActor* DamageInstigator,
    const float DamageValue
)
{
    if (!HasAuthority() || IsDead()) return;

    UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponent();
    if (!AbilitySystemComponent) return;
    if (AbilitySystemComponent->HasMatchingGameplayTag(RiftGameplayTags::State_SuperArmor_Red) ||
        AbilitySystemComponent->HasMatchingGameplayTag(RiftGameplayTags::State_Blocking))
    {
        return;
    }

    if (Reaction == ERiftPlayerHitReaction::None) return;

    Client_ShowDamageIndicator(DamageValue);

    switch (Reaction)
    {
    case ERiftPlayerHitReaction::LightHit:
        HandleLightHit(DamageInstigator);
        return;
    case ERiftPlayerHitReaction::HeavyHit:
        HandleHeavyHit(DamageInstigator);
        return;
    case ERiftPlayerHitReaction::IndicatorOnly:
    default:
        return;
    }
}

void APlayerCharacter::HandleLightHit(AActor* DamageInstigator)
{
    if (!HasAuthority() || IsDead()) return;

    UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponent();
    if (!AbilitySystemComponent) return;
    if (AbilitySystemComponent->HasMatchingGameplayTag(RiftGameplayTags::State_SuperArmor_Red) ||
        AbilitySystemComponent->HasMatchingGameplayTag(RiftGameplayTags::State_Blocking) ||
        AbilitySystemComponent->HasMatchingGameplayTag(RiftGameplayTags::State_Hit_Heavy) ||
        AbilitySystemComponent->HasMatchingGameplayTag(RiftGameplayTags::State_Hit_Light))
    {
        return;
    }

    LastHitReactDirection = DamageInstigator
        ? CalculateHitReactDirection(this, DamageInstigator->GetActorLocation())
        : ERiftHitReactDirection::Front;

    UAnimMontage* MontageToPlay = PlayerClassConfig ? PlayerClassConfig->LightHitMontage : nullptr;

    AbilitySystemComponent->CancelAllAbilities();
    AbilitySystemComponent->SetUserAbilityActivationInhibited(true);
    ActiveLightHitMontage = MontageToPlay;
    SetLightHitState(true);
    ClearActionCancelableState();

    ClearMovementInputCache();
    StopAssistedFacing();
    if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
    {
        MovementComponent->StopMovementImmediately();
    }

    Client_SetLightHitControlState(true);
    Multicast_PlayLightHit(LastHitReactDirection, 0.0f, DamageInstigator);
}

void APlayerCharacter::HandleHeavyHit(AActor* DamageInstigator)
{
    if (!HasAuthority() || IsDead()) return;

    UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponent();
    if (!AbilitySystemComponent) return;

    if (AbilitySystemComponent->HasMatchingGameplayTag(RiftGameplayTags::State_Hit_Heavy) ||
        AbilitySystemComponent->HasMatchingGameplayTag(RiftGameplayTags::State_SuperArmor_Red) ||
        AbilitySystemComponent->HasMatchingGameplayTag(RiftGameplayTags::State_Blocking))
    {
        return;
    }

    if (AbilitySystemComponent->HasMatchingGameplayTag(RiftGameplayTags::State_Hit_Light))
    {
        SetLightHitState(false);
        ActiveLightHitMontage.Reset();
    }

    UAnimMontage* MontageToPlay = PlayerClassConfig ? PlayerClassConfig->HeavyHitMontage : nullptr;
    if (!MontageToPlay) return;

    LastHitReactDirection = DamageInstigator
        ? CalculateHitReactDirection(this, DamageInstigator->GetActorLocation())
        : ERiftHitReactDirection::Front;

    AbilitySystemComponent->CancelAllAbilities();
    AbilitySystemComponent->SetUserAbilityActivationInhibited(true);
    ActiveHeavyHitMontage = MontageToPlay;
    SetHeavyHitState(true);
    ClearActionCancelableState();

    ClearMovementInputCache();
    StopAssistedFacing();
    if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
    {
        MovementComponent->StopMovementImmediately();
    }

    Client_SetHeavyHitControlState(true);
    Multicast_PlayHeavyHit(LastHitReactDirection);
}

void APlayerCharacter::FinishLightHit()
{
    if (!HasAuthority()) return;
    if (IsDead()) return;

    UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponent();
    if (!AbilitySystemComponent) return;
    if (!AbilitySystemComponent->HasMatchingGameplayTag(RiftGameplayTags::State_Hit_Light)) return;

    SetLightHitState(false);
    ActiveLightHitMontage.Reset();
    AbilitySystemComponent->SetUserAbilityActivationInhibited(false);
    Client_SetLightHitControlState(false);

    if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
    {
        if (MovementComponent->MovementMode == MOVE_None)
        {
            MovementComponent->SetMovementMode(MOVE_Walking);
        }
    }
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

void APlayerCharacter::OnLightHitMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    static_cast<void>(bInterrupted);

    if (!HasAuthority()) return;
    if (IsDead()) return;
    if (!Montage || ActiveLightHitMontage.Get() != Montage) return;

    UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponent();
    if (!AbilitySystemComponent) return;
    if (!AbilitySystemComponent->HasMatchingGameplayTag(RiftGameplayTags::State_Hit_Light)) return;

    FinishLightHit();
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

bool APlayerCharacter::IsInLightHitState() const
{
    const UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponent();
    return AbilitySystemComponent &&
        AbilitySystemComponent->HasMatchingGameplayTag(RiftGameplayTags::State_Hit_Light);
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
    static_cast<void>(DamageValue);
    static_cast<void>(DamageInstigator);

    LastHitReactDirection = Direction;
    ClearActionCancelableState();

    UAnimMontage* MontageToPlay = PlayerClassConfig ? PlayerClassConfig->LightHitMontage : nullptr;
    if (!MontageToPlay)
    {
        if (HasAuthority())
        {
            FinishLightHit();
        }
        return;
    }

    USkeletalMeshComponent* CharacterMesh = GetMesh();
    if (!CharacterMesh)
    {
        if (HasAuthority())
        {
            FinishLightHit();
        }
        return;
    }

    UAnimInstance* AnimInstance = CharacterMesh->GetAnimInstance();
    if (!AnimInstance)
    {
        if (HasAuthority())
        {
            FinishLightHit();
        }
        return;
    }

    const float MontageLength = AnimInstance->Montage_Play(MontageToPlay);
    if (MontageLength <= 0.0f)
    {
        if (HasAuthority())
        {
            FinishLightHit();
        }
        return;
    }

    AnimInstance->Montage_JumpToSection(GetHitReactSectionName(Direction), MontageToPlay);

    if (HasAuthority())
    {
        ActiveLightHitMontage = MontageToPlay;
        FOnMontageEnded EndDelegate;
        EndDelegate.BindUObject(this, &APlayerCharacter::OnLightHitMontageEnded);
        AnimInstance->Montage_SetEndDelegate(EndDelegate, MontageToPlay);
    }
}

void APlayerCharacter::Multicast_PlayHeavyHit_Implementation(const ERiftHitReactDirection Direction)
{
    LastHitReactDirection = Direction;
    ClearActionCancelableState();

    UAnimMontage* MontageToPlay = PlayerClassConfig ? PlayerClassConfig->HeavyHitMontage : nullptr;
    if (!MontageToPlay)
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

    const float MontageLength = AnimInstance->Montage_Play(MontageToPlay);
    if (MontageLength <= 0.0f)
    {
        if (HasAuthority())
        {
            FinishHeavyHit();
        }
        return;
    }

    AnimInstance->Montage_JumpToSection(GetHitReactSectionName(Direction), MontageToPlay);

    if (HasAuthority())
    {
        ActiveHeavyHitMontage = MontageToPlay;
        FOnMontageEnded EndDelegate;
        EndDelegate.BindUObject(this, &APlayerCharacter::OnHeavyHitMontageEnded);
        AnimInstance->Montage_SetEndDelegate(EndDelegate, MontageToPlay);
    }
}

void APlayerCharacter::InitPlayerProperties()
{
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
    if (PlayerClassConfig && PlayerClassConfig->AnimClass)
    {
        if (USkeletalMeshComponent* CharacterMesh = GetMesh())
        {
            CharacterMesh->SetAnimInstanceClass(PlayerClassConfig->AnimClass);
        }
    }

    ApplyMovementSettings();
    ApplyWeaponsFromConfig();
}

void APlayerCharacter::ApplyClassConfigOnAuthority()
{
    if (!HasAuthority()) return;

    ApplyCommonAttributesFromConfig();
    GrantAbilitiesFromClassConfig();
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
        URiftResourceAttributeSet::GetMaxUltimateChargeAttribute(),
        PlayerClassConfig->MaxUltimateCharge
    );
    AbilitySystemComponent->SetNumericAttributeBase(
        URiftResourceAttributeSet::GetUltimateChargeAttribute(),
        PlayerClassConfig->UltimateCharge
    );

}

void APlayerCharacter::ApplyWeaponsFromConfig()
{
    if (!PlayerClassConfig) return;
    ApplyWeapons(PlayerClassConfig->Weapons);
}

void APlayerCharacter::ClearGrantedAbilityHandles()
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
    PerfectDodgeUltimateChargeReward = 0.0f;
}

void APlayerCharacter::SetLightHitState(const bool bInLightHit)
{
    UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponent();
    if (!AbilitySystemComponent) return;

    const int32 NewCount = bInLightHit ? 1 : 0;
    AbilitySystemComponent->SetLooseGameplayTagCount(RiftGameplayTags::State_Hit_Light, NewCount);
    AbilitySystemComponent->SetReplicatedLooseGameplayTagCount(RiftGameplayTags::State_Hit_Light, NewCount);
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

    ClearGrantedAbilityHandles();

    if (!PlayerClassConfig) return;

    FLogger::Log(
        this,
        FString::Printf(TEXT("GrantAbilitiesFromClassConfig: AbilityConfigs.Num=%d"), PlayerClassConfig->AbilityConfigs.Num()),
        ELogSystem::Ability,
        ELogOutputType::LogOnly
    );

    UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponent();
    if (!AbilitySystemComponent) return;

    TSet<UClass*> GrantedAbilityClasses;

    for (URiftAbilityConfig* AbilityConfig : PlayerClassConfig->AbilityConfigs)
    {
        if (!AbilityConfig || !AbilityConfig->AbilityClass) continue;

        UClass* AbilityClass = AbilityConfig->AbilityClass.Get();
        if (!AbilityClass || GrantedAbilityClasses.Contains(AbilityClass)) continue;

        FLogger::Log(
            this,
            FString::Printf(
                TEXT("GrantAbilitiesFromClassConfig: AbilityConfig=%s AbilityClass=%s"),
                *GetNameSafe(AbilityConfig),
                *GetNameSafe(AbilityClass)
            ),
            ELogSystem::Ability,
            ELogOutputType::LogOnly
        );

        const FGameplayAbilitySpec AbilitySpec(AbilityConfig->AbilityClass, 1, INDEX_NONE, AbilityConfig);
        const FGameplayAbilitySpecHandle AbilityHandle = AbilitySystemComponent->GiveAbility(AbilitySpec);
        GrantedAbilityHandles.Add(AbilityHandle);
        GrantedAbilityClasses.Add(AbilityClass);
    }
}

void APlayerCharacter::InitMovementSettings()
{
    UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
    if (!MovementComponent) return;

    bUseControllerRotationPitch = false;
    bUseControllerRotationRoll = false;

    ApplyFacingModeToMovement();
    MovementComponent->MaxWalkSpeed = RiftPlayerDefaultRunSpeed;
    MovementComponent->MaxAcceleration = RiftPlayerDefaultMaxAcceleration;
    MovementComponent->BrakingDecelerationWalking = RiftPlayerDefaultBrakingDecelerationWalking;
    MovementComponent->bUseSeparateBrakingFriction = true;
    MovementComponent->BrakingFriction = RiftPlayerDefaultBrakingFriction;
    MovementComponent->BrakingFrictionFactor = RiftPlayerDefaultBrakingFrictionFactor;
    MovementComponent->GroundFriction = RiftPlayerDefaultGroundFriction;

    CurrentTurnRate = (RiftPlayerDefaultMinTurnRate + RiftPlayerDefaultMaxTurnRate) / 2.0f;
    MovementComponent->RotationRate = FRotator(0.0f, CurrentTurnRate, 0.0f);
}

void APlayerCharacter::ApplyCameraRelativeMovementInput()
{
    if (IsDead() ||
        IsInLightHitState() ||
        IsInHeavyHitState())
    {
        ClearMovementInputCache();
        return;
    }
    if (!Controller || MovementInputVector.IsNearlyZero()) return;

    const FVector2D InputVector = MovementInputVector.GetClampedToMaxSize(1.0f);

    const FRotator ControlRotation = Controller->GetControlRotation();

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

    MovementComponent->MaxWalkSpeed = PlayerClassConfig ? PlayerClassConfig->RunSpeed : RiftPlayerDefaultRunSpeed;
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

    const float TargetTurnRate = FMath::GetMappedRangeValueClamped(
        FVector2D(0.0f, 90.0f),
        FVector2D(RiftPlayerDefaultMinTurnRate, RiftPlayerDefaultMaxTurnRate),
        AngleDelta
    );

    const float DynamicInterpSpeed = FMath::GetMappedRangeValueClamped(
        FVector2D(0.0f, 90.0f),
        FVector2D(RiftPlayerDefaultMinTurnRateInterpSpeed, RiftPlayerDefaultMaxTurnRateInterpSpeed),
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
