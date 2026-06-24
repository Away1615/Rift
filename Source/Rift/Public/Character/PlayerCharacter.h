// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystem/RiftAttributeReactionReceiver.h"
#include "Camera/CameraComponent.h"
#include "Character/BaseCharacter.h"
#include "Combat/RiftHitReactionTypes.h"
#include "Combat/RiftPlayerHitReactionTypes.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameplayAbilitySpec.h"
#include "GameplayEffectTypes.h"
#include "TimerManager.h"
#include "PlayerCharacter.generated.h"

class UAbilitySystemComponent;
class UAnimMontage;
class USkeletalMeshComponent;
class UPlayerClassConfig;
class URiftTargetAssistComponent;
class URiftWeaponTraceComponent;
class URiftCombatFeedbackComponent;
class UPlayerAppearanceComponent;
class UNiagaraComponent;
class UNiagaraSystem;
class UGA_TwinSwordRapidSlash;

UENUM(BlueprintType)
enum class ERiftCharacterFacingMode : uint8
{
	Movement UMETA(DisplayName="Movement"),
	CombatAssist UMETA(DisplayName="Combat Assist")
};

UCLASS()
class RIFT_API APlayerCharacter : public ABaseCharacter, public IAbilitySystemInterface, public IRiftAttributeReactionReceiver
{
	GENERATED_BODY()

public:
	APlayerCharacter();

	void HandleMove(const FVector2D& InputValue);
	// Server+client combat paths only (e.g. LocalPredicted ability hooks), not local input callbacks.
	UFUNCTION(BlueprintCallable, Category="Locomotion|Facing")
	void SetFacingMode(ERiftCharacterFacingMode NewFacingMode);

	UFUNCTION(BlueprintPure, Category="Locomotion|Facing")
	ERiftCharacterFacingMode GetFacingMode() const;

	// Server+client combat paths only (e.g. LocalPredicted ability hooks), not local input callbacks.
	UFUNCTION(BlueprintCallable, Category="Locomotion|Facing")
	void StartAssistedFacing(const FRotator& TargetRotation, float Duration, float RotationSpeed);

	// Server+client combat paths only (e.g. LocalPredicted ability hooks), not local input callbacks.
	UFUNCTION(BlueprintCallable, Category="Locomotion|Facing")
	void StopAssistedFacing();

	UFUNCTION(BlueprintCallable, Category="Locomotion")
	void ClearMovementInputCache();

	FVector GetCameraRelativeMoveDirection() const;

	void ActivatePerfectDodgeWindow(const FVector& Origin, float Duration, float UltimateChargeReward);
	bool IsPerfectDodgeWindowActive() const { return bPerfectDodgeWindowActive; }
	FVector GetPerfectDodgeOrigin() const { return PerfectDodgeOrigin; }
	void HandlePerfectDodge(AActor* InstigatorEnemy);

	void HandleDeath();
	void HandleDeath(AActor* DeathInstigator);
	virtual void HandleAttributeDeath(AActor* DeathInstigator) override;
	bool IsDead() const { return bIsDead; }
	void HandlePlayerHitReaction(ERiftPlayerHitReaction Reaction, AActor* DamageInstigator, float DamageValue);
	void HandleLightHit(AActor* DamageInstigator);
	void HandleHeavyHit(AActor* DamageInstigator);
	void FinishLightHit();

	UFUNCTION(BlueprintCallable, Category="State")
	void FinishHeavyHit();

	UFUNCTION(Server, Reliable)
	void Server_FinishHeavyHit();

	UFUNCTION(BlueprintPure, Category="State")
	bool IsInLightHitState() const;

	UFUNCTION(BlueprintPure, Category="State")
	bool IsInHeavyHitState() const;

	UFUNCTION(BlueprintPure, Category="Animation")
	ERiftHitReactDirection GetLastHitReactDirection() const { return LastHitReactDirection; }

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayDeath(ERiftHitReactDirection Direction);

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_PlayLightHit(
		ERiftHitReactDirection Direction,
		float DamageValue,
		AActor* DamageInstigator
	);

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_PlayHeavyHit(ERiftHitReactDirection Direction);

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_PlayHitFeedback(
		ERiftHitReactDirection Direction,
		float DamageValue,
		AActor* DamageInstigator
	);

	UFUNCTION(Client, Reliable)
	void Client_DisableInputOnDeath();

	UFUNCTION(Client, Reliable)
	void Client_SetDeadControlState(bool bDead);

	UFUNCTION(Client, Reliable)
	void Client_ShowDamageIndicator(float DamageValue);

	UFUNCTION(Client, Reliable)
	void Client_SetLightHitControlState(bool bInLightHit);

	UFUNCTION(Client, Reliable)
	void Client_SetHeavyHitControlState(bool bInHeavyHit);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayReviveVisual();

	void ReviveAtTransform(const FTransform& ReviveTransform);

	UFUNCTION(BlueprintImplementableEvent, Category="Damage")
	void OnPlayerHitDamaged(ERiftHitReactDirection Direction, float DamageValue, AActor* DamageInstigator);

	UFUNCTION(BlueprintImplementableEvent, Category="Combat|Feedback")
	void OnDamageIndicator(float DamageValue);

	UFUNCTION(BlueprintImplementableEvent, Category="Respawn")
	void OnPlayerRevived();

	bool HasMovementInput() const;
	bool IsMovementAccelerating() const;

	virtual void Tick(float DeltaTime) override;
	virtual void PostInitializeComponents() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void UnPossessed() override;
	virtual void PawnClientRestart() override;
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	// Server-side
	virtual void PossessedBy(AController* NewController) override;

	// Client-side
	virtual void OnRep_PlayerState() override;

	UFUNCTION(BlueprintPure, Category="Class")
	UPlayerClassConfig* GetPlayerClassConfig() const { return PlayerClassConfig; }

	UFUNCTION(BlueprintCallable, Server, Reliable, Category="Class")
	void SelectPlayerClass(UPlayerClassConfig* NewPlayerClassConfig);

	UFUNCTION(BlueprintCallable, Category="Lobby|Preview")
	void ApplyClassConfigForPreview(UPlayerClassConfig* PreviewClassConfig);

	void StartRapidSlashAuraVisual(
		UNiagaraSystem* AuraNiagara,
		FName AttachSocketName,
		const FVector& LocationOffset,
		const FRotator& RotationOffset,
		const FVector& Scale
	);
	void StopRapidSlashAuraVisual();

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_StartRapidSlashAuraVisual(
		UNiagaraSystem* AuraNiagara,
		FName AttachSocketName,
		FVector LocationOffset,
		FRotator RotationOffset,
		FVector Scale
	);

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_StopRapidSlashAuraVisual();

	void RequestRapidSlashFinisher();

	UFUNCTION(Server, Reliable)
	void Server_RequestRapidSlashFinisher();

	void SetActionCancelableState(bool bCancelable);
	void ClearActionCancelableState();
	bool IsActionCancelable() const;
	bool IsDodgingForActionCancel() const;
	void CancelPlayerActionAbilities(bool bIncludeGuard, bool bIncludeDodge);

	void SetActiveTwinSwordRapidSlashAbility(UGA_TwinSwordRapidSlash* Ability);
	void ClearActiveTwinSwordRapidSlashAbility(UGA_TwinSwordRapidSlash* Ability);

	UFUNCTION(BlueprintPure, Category="Combat")
	URiftTargetAssistComponent* GetTargetAssistComponent() const { return TargetAssistComponent; }

	UFUNCTION(BlueprintPure, Category="Combat")
	URiftWeaponTraceComponent* GetWeaponTraceComponent() const { return WeaponTraceComponent; }

	UFUNCTION(BlueprintPure, Category="Combat")
	URiftCombatFeedbackComponent* GetCombatFeedbackComponent() const { return CombatFeedbackComponent; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera")
	TObjectPtr<USpringArmComponent> SpringArmComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera")
	TObjectPtr<UCameraComponent> CameraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat")
	TObjectPtr<URiftTargetAssistComponent> TargetAssistComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat")
	TObjectPtr<URiftWeaponTraceComponent> WeaponTraceComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat")
	TObjectPtr<URiftCombatFeedbackComponent> CombatFeedbackComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Appearance", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UPlayerAppearanceComponent> AppearanceComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Appearance")
	TObjectPtr<USkeletalMeshComponent> HairMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Appearance")
	TObjectPtr<USkeletalMeshComponent> ArmUpperLeftMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Appearance")
	TObjectPtr<USkeletalMeshComponent> ArmUpperRightMesh;

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

	TArray<FGameplayAbilitySpecHandle> GrantedAbilityHandles;

private:
	void InitPlayerProperties();
	void InitCameraComponents();

	void AssemblePlayerClass();
	void ApplyAppearanceFromPlayerState();
	void ApplyClassConfigOnAllRoles();
	void ApplyClassConfigOnAuthority();
	void ApplyCommonAttributesFromConfig();
	void ApplyWeaponsFromConfig();
	void ClearGrantedAbilityHandles();
	void GrantAbilitiesFromClassConfig();
	void SetLightHitState(bool bInLightHit);
	void SetHeavyHitState(bool bInHeavyHit);

	TWeakObjectPtr<UAnimMontage> ActiveLightHitMontage;
	TWeakObjectPtr<UAnimMontage> ActiveHeavyHitMontage;

	UPROPERTY(Transient)
	TObjectPtr<UNiagaraComponent> ActiveRapidSlashAuraComponent;

	TWeakObjectPtr<UGA_TwinSwordRapidSlash> ActiveTwinSwordRapidSlashAbility;

	FVector PerfectDodgeOrigin = FVector::ZeroVector;
	float PerfectDodgeUltimateChargeReward = 0.0f;
	bool bPerfectDodgeWindowActive = false;
	bool bIsDead = false;
	ERiftHitReactDirection LastHitReactDirection = ERiftHitReactDirection::Front;
	FTimerHandle PerfectDodgeWindowTimerHandle;

	UFUNCTION()
	void OnRep_PlayerClassConfig();

	UFUNCTION()
	void OnLightHitMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	UFUNCTION()
	void OnHeavyHitMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	void DeactivatePerfectDodgeWindow();

	void InitMovementSettings();
	void ApplyCameraRelativeMovementInput();
	void ApplyFacingModeToMovement();
	void ApplyMovementSettings() const;
	void UpdateMovementRotationRate(float DeltaTime);
	void UpdateAssistedFacing(float DeltaTime);
};
