// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/PlayerCharacter.h"

#include "AbilitySystem/Attributes/HealthAttributeSet.h"
#include "Player/BasePlayerState.h"
#include "Data/PlayerClassConfig.h"
#include "Data/Ability/BaseAbilityConfig.h"
#include "Data/Ability/PlayerAbilitySetConfig.h"
#include "Data/Animation/PlayerAnimationConfig.h"
#include "Data/Common/PlayerCommonConfig.h"
#include "Data/Weapon/PlayerWeaponConfig.h"
#include "AbilitySystem/GameplayAbilities/BaseGameplayAbility.h"
#include "Components/SkeletalMeshComponent.h"
#include "Debug/Logger.h"
#include "Equipment/PlayerWeapon.h"
#include "GameFramework/CharacterMovementComponent.h"

APlayerCharacter::APlayerCharacter()
{
    InitPlayerProperties();
    InitMovementSettings();
    InitCameraComponents();
}

void APlayerCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    ApplyCameraRelativeMovementInput();
    UpdateMovementRotationRate(DeltaTime);
}

void APlayerCharacter::PostInitializeComponents()
{
    Super::PostInitializeComponents();
    ApplyAnimationConfig();
    ApplyMovementTuningSettings();
}

UAbilitySystemComponent* APlayerCharacter::GetAbilitySystemComponent() const
{
    const ABasePlayerState* BasePlayerState = GetPlayerState<ABasePlayerState>();
    return BasePlayerState
        ? BasePlayerState->GetAbilitySystemComponent()
        : Super::GetAbilitySystemComponent();
}

UPlayerAnimationConfig* APlayerCharacter::GetPlayerAnimationConfig() const
{
    return PlayerClassConfig ? PlayerClassConfig->PlayerAnimationConfig : nullptr;
}

TArray<APlayerWeapon*> APlayerCharacter::GetEquippedWeapons() const
{
    TArray<APlayerWeapon*> Weapons;
    Weapons.Reserve(EquippedWeapons.Num());

    for (const TObjectPtr<APlayerWeapon>& Weapon : EquippedWeapons)
    {
        if (Weapon)
        {
            Weapons.Add(Weapon.Get());
        }
    }

    return Weapons;
}

void APlayerCharacter::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);
    InitGasActorInfo();
    InitAttributesFromConfig();
    EquipWeaponsFromConfig();
    GrantClassAbilities();
}

void APlayerCharacter::OnRep_PlayerState()
{
    Super::OnRep_PlayerState();
    InitGasActorInfo();
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

void APlayerCharacter::HandleMove(const FVector2D& InputValue)
{
    MovementInputVector = InputValue;

    if (InputValue.IsNearlyZero())
    {
        ClearMovementInput();
    }
}

void APlayerCharacter::ToggleWalkRun()
{
    bWantsToRun = !bWantsToRun;
    ApplyMovementTuningSettings();
}

bool APlayerCharacter::IsRunning() const
{
    return bWantsToRun;
}

void APlayerCharacter::InitPlayerProperties()
{
    // Network
    bReplicates = true;
    SetReplicateMovement(true);
}

void APlayerCharacter::InitGasActorInfo()
{
    ABasePlayerState* BasePlayerState = GetPlayerState<ABasePlayerState>();
    if (BasePlayerState && BasePlayerState->GetAbilitySystemComponent())
    {
        BasePlayerState->GetAbilitySystemComponent()
                       ->InitAbilityActorInfo(BasePlayerState, this);
    }
}

void APlayerCharacter::InitCameraComponents()
{
    SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComponent"));
    SpringArmComponent->SetupAttachment(RootComponent);
    SpringArmComponent->bUsePawnControlRotation = true;

    CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
    CameraComponent->SetupAttachment(SpringArmComponent);
    CameraComponent->bUsePawnControlRotation = false;
}

void APlayerCharacter::InitAttributesFromConfig()
{
    if (!HasAuthority() || !PlayerClassConfig || !PlayerClassConfig->PlayerCommonConfig) return;

    const ABasePlayerState* BasePlayerState = GetPlayerState<ABasePlayerState>();
    if (!BasePlayerState) return;

    UHealthAttributeSet* HealthSet = BasePlayerState->GetHealthAttributeSet();
    if (!HealthSet) return;

    const UPlayerCommonConfig* CommonConfig = PlayerClassConfig->PlayerCommonConfig;

    HealthSet->SetMaxHealth(CommonConfig->MaxHealth);
    HealthSet->SetHealth(FMath::Clamp(CommonConfig->Health, 0.0f, CommonConfig->MaxHealth));
}

void APlayerCharacter::ApplyAnimationConfig()
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

void APlayerCharacter::EquipWeaponsFromConfig()
{
    if (!HasAuthority()) return;

    ClearEquippedWeapons();

    if (!PlayerClassConfig)
    {
        Logger::Error(this, TEXT("EquipWeapons failed: PlayerClassConfig is missing"));
        return;
    }

    const UPlayerWeaponConfig* WeaponConfig = PlayerClassConfig->PlayerWeaponConfig;
    if (!WeaponConfig)
    {
        Logger::Error(this, TEXT("EquipWeapons failed: PlayerWeaponConfig is missing"));
        return;
    }

    for (const FPlayerWeaponPartConfig& WeaponPartConfig : WeaponConfig->EquippedWeapons)
    {
        APlayerWeapon* WeaponActor = SpawnAndAttachWeapon(WeaponPartConfig);
        if (WeaponActor)
        {
            EquippedWeapons.Add(WeaponActor);
        }
    }
}

void APlayerCharacter::ClearEquippedWeapons()
{
    for (const TObjectPtr<APlayerWeapon>& EquippedWeapon : EquippedWeapons)
    {
        if (EquippedWeapon)
        {
            EquippedWeapon->Destroy();
        }
    }

    EquippedWeapons.Empty();
}

APlayerWeapon* APlayerCharacter::SpawnAndAttachWeapon(const FPlayerWeaponPartConfig& WeaponPartConfig)
{
    USkeletalMeshComponent* CharacterMesh = GetMesh();
    if (!CharacterMesh)
    {
        Logger::Error(this, TEXT("EquipWeapons failed: Character mesh is missing"));
        return nullptr;
    }

    if (!WeaponPartConfig.WeaponActorClass)
    {
        Logger::Error(this, TEXT("EquipWeapons skipped: WeaponActorClass is missing"));
        return nullptr;
    }

    const FName SocketName = WeaponPartConfig.AttachSocket;
    if (SocketName.IsNone())
    {
        Logger::Error(this, TEXT("EquipWeapons skipped: AttachSocket is None"));
        return nullptr;
    }

    if (!CharacterMesh->DoesSocketExist(SocketName))
    {
        Logger::Error(this, FString::Printf(TEXT("EquipWeapons skipped: socket %s does not exist"), *SocketName.ToString()));
        return nullptr;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.Instigator = this;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    APlayerWeapon* WeaponActor = GetWorld()->SpawnActor<APlayerWeapon>(
        WeaponPartConfig.WeaponActorClass,
        FTransform::Identity,
        SpawnParams
    );

    if (!WeaponActor)
    {
        Logger::Error(this, TEXT("EquipWeapons failed: SpawnActor returned null"));
        return nullptr;
    }

    WeaponActor->AttachToComponent(
        CharacterMesh,
        FAttachmentTransformRules::SnapToTargetNotIncludingScale,
        SocketName
    );

    Logger::Log(this, FString::Printf(
        TEXT("Equipped %s on %s"),
        *GetNameSafe(WeaponActor),
        *SocketName.ToString()
    ));

    return WeaponActor;
}

void APlayerCharacter::InitMovementSettings()
{
    UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
    if (!MovementComponent) return;

    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;

    // Always face to move direction
    MovementComponent->bOrientRotationToMovement = true;
    // Turn character to controller desired direction
    MovementComponent->bUseControllerDesiredRotation = false;

    const UPlayerCommonConfig* CommonConfig = PlayerClassConfig ? PlayerClassConfig->PlayerCommonConfig : nullptr;
    const float InitialTurnRate = CommonConfig
        ? (CommonConfig->MinTurnRate + CommonConfig->MaxTurnRate) / 2.0f
        : CurrentTurnRate;

    MovementComponent->RotationRate = FRotator(0.0f, InitialTurnRate, 0.0f);
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

void APlayerCharacter::ClearMovementInput()
{
    MovementInputVector = FVector2D::ZeroVector;
}

void APlayerCharacter::ApplyMovementTuningSettings() const
{
    UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
    if (!MovementComponent) return;

    const UPlayerCommonConfig* CommonConfig = PlayerClassConfig ? PlayerClassConfig->PlayerCommonConfig : nullptr;
    const float ConfigWalkSpeed = CommonConfig ? CommonConfig->WalkSpeed : 200.0f;
    const float ConfigRunSpeed = CommonConfig ? CommonConfig->RunSpeed : 600.0f;

    MovementComponent->MaxWalkSpeed = bWantsToRun ? ConfigRunSpeed : ConfigWalkSpeed;
}

void APlayerCharacter::UpdateMovementRotationRate(float DeltaTime)
{
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

void APlayerCharacter::GrantClassAbilities() const
{
    if (!HasAuthority()) return;

    UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
    if (!ASC || !PlayerClassConfig) return;

    const UPlayerAbilitySetConfig* AbilityConfig = PlayerClassConfig->PlayerAbilityConfig;
    if (!AbilityConfig) return;

    GiveConfiguredAbility(ASC, AbilityConfig->CoreAbilityConfig);
    GiveConfiguredAbility(ASC, AbilityConfig->PrimaryAbilityConfig);
    // GiveConfiguredAbility(ASC, PlayerClassConfig->SecondaryAbilityConfig);
    // GiveConfiguredAbility(ASC, PlayerClassConfig->Skill1AbilityConfig);
    // GiveConfiguredAbility(ASC, PlayerClassConfig->Skill2AbilityConfig);
    // GiveConfiguredAbility(ASC, PlayerClassConfig->UltimateAbilityConfig);

    for (const TSubclassOf<UBaseGameplayAbility> AbilityClass : AbilityConfig->PassiveAbilities)
    {
        GiveAbilityFromClass(ASC, AbilityClass, PlayerClassConfig);
    }
}

void APlayerCharacter::GiveConfiguredAbility(UAbilitySystemComponent* ASC, UBaseAbilityConfig* AbilityConfig)
{
    if (!AbilityConfig) return;

    GiveAbilityFromClass(ASC, AbilityConfig->AbilityClass, AbilityConfig);
}

void APlayerCharacter::GiveAbilityFromClass(UAbilitySystemComponent* ASC, TSubclassOf<UBaseGameplayAbility> AbilityClass, UObject* SourceObject)
{
    if (!ASC || !AbilityClass || ASC->FindAbilitySpecFromClass(AbilityClass)) return;

    const UBaseGameplayAbility* AbilityCDO = AbilityClass->GetDefaultObject<UBaseGameplayAbility>();

    if (!AbilityCDO) return;

    ASC->GiveAbility(FGameplayAbilitySpec(
        AbilityClass,
        1,
        static_cast<int32>(AbilityCDO->AbilityInputID),
        SourceObject
    ));
}
