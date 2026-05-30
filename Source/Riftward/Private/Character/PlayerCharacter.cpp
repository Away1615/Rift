// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/PlayerCharacter.h"

#include "AbilitySystemComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameplayTags/RiftwardGameplayTags.h"
#include "Player/BasePlayerState.h"

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
}

void APlayerCharacter::PostInitializeComponents()
{
    Super::PostInitializeComponents();
    ApplyMovementTuningSettings();
}

UAbilitySystemComponent* APlayerCharacter::GetAbilitySystemComponent() const
{
    const ABasePlayerState* BasePlayerState = GetPlayerState<ABasePlayerState>();
    return BasePlayerState
        ? BasePlayerState->GetAbilitySystemComponent()
        : Super::GetAbilitySystemComponent();
}

void APlayerCharacter::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);
    InitGasActorInfo();
}

void APlayerCharacter::OnRep_PlayerState()
{
    Super::OnRep_PlayerState();
    InitGasActorInfo();
}

void APlayerCharacter::HandleMove(const FVector2D& InputValue)
{
    if (!CanAcceptGroundedActions())
    {
        ClearMovementInput();
        return;
    }

    MovementInputVector = InputValue;
    bHasMovementInput = !InputValue.IsNearlyZero();

    if (!bHasMovementInput)
    {
        ClearMovementInput();
    }
}

void APlayerCharacter::HandleJumpStarted()
{
    if (!CanJump())
    {
        return;
    }

    SetAirborneState(true);
    ClearMovementInput();
    Jump();
}

void APlayerCharacter::HandleJumpCompleted()
{
    StopJumping();
}

bool APlayerCharacter::CanAcceptGroundedActions() const
{
    const UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
    const UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponent();

    return MovementComponent
        && MovementComponent->IsMovingOnGround()
        && (!AbilitySystemComponent || !AbilitySystemComponent->HasMatchingGameplayTag(RiftwardGameplayTags::State_Movement_Airborne));
}

void APlayerCharacter::Landed(const FHitResult& Hit)
{
    Super::Landed(Hit);
    SetAirborneState(false);
}

void APlayerCharacter::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode)
{
    Super::OnMovementModeChanged(PrevMovementMode, PreviousCustomMode);

    const UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
    SetAirborneState(MovementComponent && MovementComponent->IsFalling());
}

void APlayerCharacter::InitPlayerProperties()
{
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;

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

void APlayerCharacter::InitMovementSettings() const
{
    UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
    if (!MovementComponent) return;

    MovementComponent->bOrientRotationToMovement = true;
    MovementComponent->bUseControllerDesiredRotation = false;
    MovementComponent->AirControl = 0.0f;
}

void APlayerCharacter::ApplyMovementTuningSettings() const
{
    UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
    if (!MovementComponent) return;

    MovementComponent->RotationRate = MovementRotationRate;
    MovementComponent->JumpZVelocity = JumpZVelocity;
    MovementComponent->GravityScale = GravityScale;
}

void APlayerCharacter::ApplyCameraRelativeMovementInput()
{
    if (!Controller || !bHasMovementInput || !CanAcceptGroundedActions())
    {
        return;
    }

    const FVector2D InputVector = MovementInputVector.GetClampedToMaxSize(1.0f);
    const FRotator ControlRotation = Controller->GetControlRotation();
    const FRotator YawRotation(0.0f, ControlRotation.Yaw, 0.0f);

    const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
    const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
    const FVector MoveVector = ForwardDirection * InputVector.Y + RightDirection * InputVector.X;

    WorldMoveDirection = MoveVector.GetSafeNormal();
    AddMovementInput(WorldMoveDirection, MoveVector.Size());
}

void APlayerCharacter::ClearMovementInput()
{
    MovementInputVector = FVector2D::ZeroVector;
    bHasMovementInput = false;
    WorldMoveDirection = FVector::ZeroVector;
}

void APlayerCharacter::SetAirborneState(bool bIsAirborne) const
{
    UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponent();
    if (!AbilitySystemComponent) return;

    AbilitySystemComponent->SetLooseGameplayTagCount(RiftwardGameplayTags::State_Movement_Airborne, bIsAirborne ? 1 : 0);
}
