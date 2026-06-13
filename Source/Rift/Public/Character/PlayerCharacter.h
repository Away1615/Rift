// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "Camera/CameraComponent.h"
#include "Character/BaseCharacter.h"
#include "Combat/RiftWeaponTraceComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameplayAbilitySpec.h"
#include "TimerManager.h"
#include "PlayerCharacter.generated.h"

class UPlayerAnimationConfig;
class UPlayerClassConfig;
class UAbilitySystemComponent;
class URiftTargetAssistComponent;
class URiftWeaponTraceComponent;
class UStaticMeshComponent;
struct FPlayerWeaponPartConfig;

UENUM(BlueprintType)
enum class ERiftCharacterFacingMode : uint8
{
	Movement UMETA(DisplayName="Movement"),
	CombatAssist UMETA(DisplayName="Combat Assist")
};

/**
 *
 */
UCLASS()
class RIFT_API APlayerCharacter : public ABaseCharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	APlayerCharacter();

	void HandleMove(const FVector2D& InputValue);
	// Only call from combat state paths that execute on both server and owning client,
	// such as a future LocalPredicted GameplayAbility ActivateAbility/EndAbility.
	// Do not call from purely local input callbacks.
	UFUNCTION(BlueprintCallable, Category="Locomotion|Facing")
	void SetFacingMode(ERiftCharacterFacingMode NewFacingMode);

	UFUNCTION(BlueprintPure, Category="Locomotion|Facing")
	ERiftCharacterFacingMode GetFacingMode() const;

	// Only call from combat state paths that execute on both server and owning client,
	// such as a future LocalPredicted GameplayAbility ActivateAbility/EndAbility.
	// Do not call from purely local input callbacks.
	UFUNCTION(BlueprintCallable, Category="Locomotion|Facing")
	void StartAssistedFacing(const FRotator& TargetRotation, float Duration, float RotationSpeed);

	// Only call from combat state paths that execute on both server and owning client,
	// such as a future LocalPredicted GameplayAbility ActivateAbility/EndAbility.
	// Do not call from purely local input callbacks.
	UFUNCTION(BlueprintCallable, Category="Locomotion|Facing")
	void StopAssistedFacing();

	UFUNCTION(BlueprintCallable, Category="Locomotion")
	void ClearMovementInputCache();

	// Whether Player press WASD
	bool HasMovementInput() const;
	// Whether Player is Moving
	bool IsMovementAccelerating() const;

	virtual void Tick(float DeltaTime) override;
	virtual void PostInitializeComponents() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void UnPossessed() override;
	virtual void PawnClientRestart() override;
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	// On the server, called when this Pawn is controlled by the Controller
	virtual void PossessedBy(AController* NewController) override;

	// On the client, called when PlayerState is copied to this Pawn
	virtual void OnRep_PlayerState() override;

	UFUNCTION(BlueprintPure, Category="Class")
	UPlayerClassConfig* GetPlayerClassConfig() const { return PlayerClassConfig; }

	UFUNCTION(BlueprintCallable, Server, Reliable, Category="Class")
	void SelectPlayerClass(UPlayerClassConfig* NewPlayerClassConfig);

	UFUNCTION(BlueprintPure, Category="Animation")
	UPlayerAnimationConfig* GetPlayerAnimationConfig() const;

	UFUNCTION(BlueprintPure, Category="Weapon")
	UStaticMeshComponent* GetWeaponMeshComponent(ERiftWeaponSlot Slot) const;

	UFUNCTION(BlueprintPure, Category="Weapon")
	float GetWeaponTraceRadius(ERiftWeaponSlot Slot) const;

	UFUNCTION(BlueprintPure, Category="Combat")
	URiftTargetAssistComponent* GetTargetAssistComponent() const { return TargetAssistComponent; }

	UFUNCTION(BlueprintPure, Category="Combat")
	URiftWeaponTraceComponent* GetWeaponTraceComponent() const { return WeaponTraceComponent; }

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_PlayMeleeHitFeedback(AActor* HitEnemy);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera")
	TObjectPtr<USpringArmComponent> SpringArmComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera")
	TObjectPtr<UCameraComponent> CameraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat")
	TObjectPtr<URiftTargetAssistComponent> TargetAssistComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat")
	TObjectPtr<URiftWeaponTraceComponent> WeaponTraceComponent;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Locomotion")
	FVector2D MovementInputVector = FVector2D::ZeroVector;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Locomotion|Turn")
	float CurrentTurnRate = 600.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Locomotion|Facing")
	ERiftCharacterFacingMode FacingMode = ERiftCharacterFacingMode::Movement;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Locomotion|Facing")
	bool bIsAssistedFacing = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Locomotion|Facing")
	FRotator AssistedFacingTargetRotation = FRotator::ZeroRotator;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Locomotion|Facing")
	float AssistedFacingTimeRemaining = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Locomotion|Facing")
	float AssistedFacingRotationSpeed = 1200.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, ReplicatedUsing=OnRep_PlayerClassConfig, Category="Class")
	TObjectPtr<UPlayerClassConfig> PlayerClassConfig;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Weapon")
	TObjectPtr<UStaticMeshComponent> LeftWeaponMesh;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Weapon")
	TObjectPtr<UStaticMeshComponent> RightWeaponMesh;

	float LeftWeaponTraceRadius = 15.0f;
	float RightWeaponTraceRadius = 15.0f;

	TArray<FGameplayAbilitySpecHandle> GrantedAbilityHandles;

private:
	// Set up network replication
	void InitPlayerProperties();
	void InitCameraComponents();

	void AssemblePlayerClass();
	void ApplyAnimationConfig() const;
	void ApplyWeaponsFromConfig();
	void ClearEquippedWeapons();
	void ClearGrantedAbilities();
	void GrantAbilitiesFromClassConfig();
	void CreateAndAttachWeaponMesh(const FPlayerWeaponPartConfig& WeaponPartConfig);
	static FName GetWeaponAttachSocketName(ERiftWeaponSlot WeaponSlot);

	// Applies hit stop to one actor and restores CustomTimeDilation with a world timer.
	void ApplyHitStopToActor(AActor* TargetActor, float TimeDilation, float Duration);

	// key=Actor, value=restore timer handle
	TMap<TWeakObjectPtr<AActor>, FTimerHandle> HitStopTimerHandles;

	UFUNCTION()
	void OnRep_PlayerClassConfig();

	/* Movement and Camera Control */
	void InitMovementSettings();
	void ApplyCameraRelativeMovementInput();
	void ApplyFacingModeToMovement();
	void ApplyMovementSettings() const;
	void UpdateMovementRotationRate(float DeltaTime);
	void UpdateAssistedFacing(float DeltaTime);
};
