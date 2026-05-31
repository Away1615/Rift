// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/PlayerCharacter.h"

#include "GameFramework/CharacterMovementComponent.h"
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
    MovementInputVector = InputValue;
    bHasMovementInput = !InputValue.IsNearlyZero();

    if (!bHasMovementInput)
    {
        ClearMovementInput();
    }
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
}

void APlayerCharacter::ApplyMovementTuningSettings() const
{
    UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
    if (!MovementComponent) return;

    MovementComponent->RotationRate = MovementRotationRate;
}

void APlayerCharacter::ApplyCameraRelativeMovementInput()
{
    if (!Controller || !bHasMovementInput)
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
