// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/PlayerCharacter.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystem/Attributes/RiftPlayerAttributeSet.h"
#include "AbilitySystem/Attributes/RiftResourceAttributeSet.h"
#include "AbilitySystem/Effects/GE_StaminaRegen.h"
#include "AbilitySystem/RiftGameplayTags.h"
#include "Abilities/GameplayAbility.h"
#include "Combat/RiftCombatFeedbackComponent.h"
#include "Combat/RiftTargetAssistComponent.h"
#include "Combat/RiftWeaponTraceComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Data/Player/PlayerClassConfig.h"
#include "Data/Player/Animation/PlayerAnimationConfig.h"
#include "Data/Player/Common/PlayerCommonConfig.h"
#include "Data/Player/Combat/PlayerCombatConfig.h"
#include "Data/Player/Weapon/PlayerWeaponConfig.h"
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
    InitCameraComponents();
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
    ApplyAnimationConfig();
    ApplyMovementSettings();
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

    AssemblePlayerClass();
}

void APlayerCharacter::OnRep_PlayerState()
{
    Super::OnRep_PlayerState();

    if (UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponent())
    {
        AbilitySystemComponent->InitAbilityActorInfo(GetPlayerState(), this);
    }
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

UStaticMeshComponent* APlayerCharacter::GetWeaponMeshComponent(const ERiftWeaponSlot Slot) const
{
    switch (Slot)
    {
    case ERiftWeaponSlot::Left:
        return LeftWeaponMesh;
    case ERiftWeaponSlot::Right:
        return RightWeaponMesh;
    default:
        return nullptr;
    }
}

float APlayerCharacter::GetWeaponTraceRadius(const ERiftWeaponSlot Slot) const
{
    switch (Slot)
    {
    case ERiftWeaponSlot::Left:
        return LeftWeaponTraceRadius;
    case ERiftWeaponSlot::Right:
        return RightWeaponTraceRadius;
    default:
        return 15.0f;
    }
}

void APlayerCharacter::HandleMove(const FVector2D& InputValue)
{
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
    ApplyAnimationConfig();
    ApplyMovementSettings();

    if (!HasAuthority()) return;

    ApplyCommonAttributesFromConfig();
    GrantAbilitiesFromClassConfig();
    ApplyWeaponsFromConfig();
}

void APlayerCharacter::OnRep_PlayerClassConfig()
{
    ApplyAnimationConfig();
    ApplyMovementSettings();
    ApplyWeaponsFromConfig();
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

    const UPlayerCommonConfig* CommonConfig = PlayerClassConfig ? PlayerClassConfig->PlayerCommonConfig : nullptr;
    if (!CommonConfig) return;

    UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponent();
    if (!AbilitySystemComponent) return;

    AbilitySystemComponent->SetNumericAttributeBase(
        URiftPlayerAttributeSet::GetMaxHealthAttribute(),
        CommonConfig->MaxHealth
    );
    AbilitySystemComponent->SetNumericAttributeBase(
        URiftPlayerAttributeSet::GetHealthAttribute(),
        CommonConfig->Health
    );
    AbilitySystemComponent->SetNumericAttributeBase(
        URiftPlayerAttributeSet::GetMaxStaminaAttribute(),
        CommonConfig->MaxStamina
    );
    AbilitySystemComponent->SetNumericAttributeBase(
        URiftPlayerAttributeSet::GetStaminaAttribute(),
        CommonConfig->Stamina
    );
    AbilitySystemComponent->SetNumericAttributeBase(
        URiftResourceAttributeSet::GetMaxSwordIntentAttribute(),
        CommonConfig->MaxSwordIntent
    );
    AbilitySystemComponent->SetNumericAttributeBase(
        URiftResourceAttributeSet::GetSwordIntentAttribute(),
        CommonConfig->SwordIntent
    );
    AbilitySystemComponent->SetNumericAttributeBase(
        URiftResourceAttributeSet::GetMaxUltimateChargeAttribute(),
        CommonConfig->MaxUltimateCharge
    );
    AbilitySystemComponent->SetNumericAttributeBase(
        URiftResourceAttributeSet::GetUltimateChargeAttribute(),
        CommonConfig->UltimateCharge
    );

    if (StaminaRegenEffectHandle.IsValid())
    {
        AbilitySystemComponent->RemoveActiveGameplayEffect(StaminaRegenEffectHandle);
        StaminaRegenEffectHandle = FActiveGameplayEffectHandle();
    }

    const float RegenPerTick = CommonConfig->StaminaRegenRate * 0.1f;
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
    ClearEquippedWeapons();

    if (!PlayerClassConfig)
    {
        FLogger::Error(this, TEXT("EquipWeapons failed: PlayerClassConfig is missing"), ELogSystem::Weapon);
        return;
    }

    const UPlayerWeaponConfig* WeaponConfig = PlayerClassConfig->PlayerWeaponConfig;
    if (!WeaponConfig)
    {
        FLogger::Error(this, TEXT("EquipWeapons failed: PlayerWeaponConfig is missing"), ELogSystem::Weapon);
        return;
    }

    for (const FPlayerWeaponPartConfig& WeaponPartConfig : WeaponConfig->EquippedWeapons)
    {
        CreateAndAttachWeaponMesh(WeaponPartConfig);
    }
}

void APlayerCharacter::ClearEquippedWeapons()
{
    if (LeftWeaponMesh)
    {
        LeftWeaponMesh->DestroyComponent();
        LeftWeaponMesh = nullptr;
    }

    if (RightWeaponMesh)
    {
        RightWeaponMesh->DestroyComponent();
        RightWeaponMesh = nullptr;
    }

    LeftWeaponTraceRadius = 15.0f;
    RightWeaponTraceRadius = 15.0f;
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

void APlayerCharacter::CreateAndAttachWeaponMesh(const FPlayerWeaponPartConfig& WeaponPartConfig)
{
    USkeletalMeshComponent* CharacterMesh = GetMesh();
    if (!CharacterMesh)
    {
        FLogger::Error(this, TEXT("EquipWeapons failed: Character mesh is missing"), ELogSystem::Weapon);
        return;
    }

    if (!WeaponPartConfig.WeaponMesh)
    {
        FLogger::Error(this, TEXT("EquipWeapons skipped: WeaponMesh is missing"), ELogSystem::Weapon);
        return;
    }

    const FName SocketName = GetWeaponAttachSocketName(WeaponPartConfig.WeaponSlot);
    if (SocketName.IsNone())
    {
        FLogger::Error(this, TEXT("EquipWeapons skipped: WeaponSlot has no mapped socket"), ELogSystem::Weapon);
        return;
    }

    if (!CharacterMesh->DoesSocketExist(SocketName))
    {
        FLogger::Error(this, FString::Printf(TEXT("EquipWeapons skipped: socket %s does not exist"), *SocketName.ToString()), ELogSystem::Weapon);
        return;
    }

    UStaticMeshComponent* WeaponMeshComponent = NewObject<UStaticMeshComponent>(this);

    WeaponMeshComponent->SetStaticMesh(WeaponPartConfig.WeaponMesh);
    WeaponMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    WeaponMeshComponent->SetGenerateOverlapEvents(false);
    WeaponMeshComponent->AttachToComponent(
        CharacterMesh,
        FAttachmentTransformRules::SnapToTargetIncludingScale,
        SocketName
    );
    WeaponMeshComponent->RegisterComponent();

    if (WeaponPartConfig.WeaponSlot == ERiftWeaponSlot::Left)
    {
        LeftWeaponMesh = WeaponMeshComponent;
        LeftWeaponTraceRadius = WeaponPartConfig.TraceRadius;
    }
    else if (WeaponPartConfig.WeaponSlot == ERiftWeaponSlot::Right)
    {
        RightWeaponMesh = WeaponMeshComponent;
        RightWeaponTraceRadius = WeaponPartConfig.TraceRadius;
    }

    FLogger::Log(this, FString::Printf(
        TEXT("Equipped weapon mesh %s on %s"),
        *GetNameSafe(WeaponPartConfig.WeaponMesh),
        *SocketName.ToString()
    ), ELogSystem::Weapon);
}

FName APlayerCharacter::GetWeaponAttachSocketName(const ERiftWeaponSlot WeaponSlot)
{
    switch (WeaponSlot)
    {
    case ERiftWeaponSlot::Left:
        return TEXT("Weapon_L");
    case ERiftWeaponSlot::Right:
        return TEXT("Weapon_R");
    default:
        return NAME_None;
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

    const UPlayerCommonConfig* CommonConfig = PlayerClassConfig ? PlayerClassConfig->PlayerCommonConfig : nullptr;

    if (!CommonConfig) return;
    MovementComponent->MaxWalkSpeed = CommonConfig->RunSpeed;

    const float InitialTurnRate = (CommonConfig->MinTurnRate + CommonConfig->MaxTurnRate) / 2.0f;
    MovementComponent->RotationRate = FRotator(0.0f, InitialTurnRate, 0.0f);

    MovementComponent->MaxAcceleration = CommonConfig->MaxAcceleration;
    MovementComponent->BrakingDecelerationWalking = CommonConfig->BrakingDecelerationWalking;
    MovementComponent->bUseSeparateBrakingFriction = true;
    MovementComponent->BrakingFriction = CommonConfig->BrakingFriction;
    MovementComponent->BrakingFrictionFactor = CommonConfig->BrakingFrictionFactor;
    MovementComponent->GroundFriction = CommonConfig->GroundFriction;
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

    const UPlayerCommonConfig* CommonConfig = PlayerClassConfig ? PlayerClassConfig->PlayerCommonConfig : nullptr;
    const float ConfigMinTurnRate = CommonConfig ? CommonConfig->MinTurnRate : 300.0f;
    const float ConfigMaxTurnRate = CommonConfig ? CommonConfig->MaxTurnRate : 1440.0f;
    const float ConfigMinTurnRateInterpSpeed = CommonConfig ? CommonConfig->MinTurnRateInterpSpeed : 3.0f;
    const float ConfigMaxTurnRateInterpSpeed = CommonConfig ? CommonConfig->MaxTurnRateInterpSpeed : 18.0f;

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
