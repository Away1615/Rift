// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/PlayerCharacter.h"

#include "AbilitySystem/Abilities/GA_Player_Sprint.h"
#include "AbilitySystem/BaseGameplayAbility.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameplayTags/RiftGameplayTags.h"
#include "Player/BasePlayerState.h"

APlayerCharacter::APlayerCharacter()
{
    StartupAbilityClasses.Add(UGA_Player_Sprint::StaticClass());

    InitPlayerProperties();
    InitMovementSettings();
    InitCameraComponents();
}

void APlayerCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    ApplyCameraRelativeMovementInput();
    RefreshMovementStateTags();
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
    GrantStartupAbilities();
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

void APlayerCharacter::SetSprinting(bool bNewSprinting)
{
    bIsSprinting = bNewSprinting;

    UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
    if (!MovementComponent) return;

    MovementComponent->MaxWalkSpeed = bIsSprinting ? SprintSpeed : WalkSpeed;
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

    MovementComponent->bOrientRotationToMovement = false;
    MovementComponent->bUseControllerDesiredRotation = true;
}

void APlayerCharacter::ApplyMovementTuningSettings() const
{
    UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
    if (!MovementComponent) return;

    MovementComponent->RotationRate = MovementRotationRate;
    MovementComponent->MaxWalkSpeed = bIsSprinting ? SprintSpeed : WalkSpeed;
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

void APlayerCharacter::RefreshMovementStateTags()
{
    UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
    if (!ASC) return;

    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (!MoveComp) return;

    const FRiftGameplayTags& RiftTags = FRiftGameplayTags::Get();

    const bool bAirborne = MoveComp->IsFalling();

    ASC->SetLooseGameplayTagCount(RiftTags.State_Movement_Airborne, bAirborne ? 1 : 0);
    ASC->SetLooseGameplayTagCount(RiftTags.State_Movement_Grounded, bAirborne ? 0 : 1);
}

void APlayerCharacter::GrantStartupAbilities()
{
    if (!HasAuthority()) return;

    UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
    if (!ASC) return;

    for (const TSubclassOf<UBaseGameplayAbility> AbilityClass : StartupAbilityClasses)
    {
        if (!AbilityClass || ASC->FindAbilitySpecFromClass(AbilityClass))
        {
            continue;
        }

        const UBaseGameplayAbility* AbilityCDO = AbilityClass->GetDefaultObject<UBaseGameplayAbility>();
        if (!AbilityCDO)
        {
            continue;
        }

        ASC->GiveAbility(FGameplayAbilitySpec(
            AbilityClass,
            1,
            static_cast<int32>(AbilityCDO->AbilityInputID),
            this
            ));
    }
}
