// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/PlayerCharacter.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "Player/BasePlayerState.h"

APlayerCharacter::APlayerCharacter()
{
    InitProperties();
    InitCharacterMovementComponent();
    InitCameraComponent();
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
    InitGASActorInfo();
}

void APlayerCharacter::OnRep_PlayerState()
{
    Super::OnRep_PlayerState();
    InitGASActorInfo();
}

void APlayerCharacter::HandleMove(const FVector2D& InputValue)
{
    MovementInputVector = InputValue;
    bHasMovementInput = !InputValue.IsNearlyZero();

    if (!Controller || !bHasMovementInput)
    {
        WorldMoveDirection = FVector::ZeroVector;
        return;
    }

    const FRotator ControlRotation = Controller->GetControlRotation();
    const FRotator YawRotation(0.0f, ControlRotation.Yaw, 0.0f);

    const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
    const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

    WorldMoveDirection = (ForwardDirection * InputValue.Y + RightDirection * InputValue.X).GetSafeNormal();

    AddMovementInput(ForwardDirection, InputValue.Y);
    AddMovementInput(RightDirection, InputValue.X);
}

void APlayerCharacter::HandleJumpStarted()
{
    Jump();
}

void APlayerCharacter::HandleJumpCompleted()
{
    StopJumping();
}

void APlayerCharacter::InitProperties()
{
    // Third-Person Rotation
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;

    // Network
    bReplicates = true;
    SetReplicateMovement(true);
}

void APlayerCharacter::InitGASActorInfo()
{
    ABasePlayerState* BasePlayerState = GetPlayerState<ABasePlayerState>();
    if (BasePlayerState && BasePlayerState->GetAbilitySystemComponent())
    {
        BasePlayerState->GetAbilitySystemComponent()
                       ->InitAbilityActorInfo(BasePlayerState, this);
    }
}

void APlayerCharacter::InitCameraComponent()
{
    SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComponent"));
    SpringArmComponent->SetupAttachment(RootComponent);
    SpringArmComponent->bUsePawnControlRotation = true;

    CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
    CameraComponent->SetupAttachment(SpringArmComponent);
    CameraComponent->bUsePawnControlRotation = false;
}

void APlayerCharacter::InitCharacterMovementComponent()
{
    UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
    if (!MovementComponent) return;

    MovementComponent->bOrientRotationToMovement = true;
    MovementComponent->bUseControllerDesiredRotation = false;
    MovementComponent->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
}
