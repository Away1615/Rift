// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraComponent.h"
#include "Character/BaseCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "PlayerCharacter.generated.h"

class APlayerWeapon;
class UBaseAbilityConfig;
class UPlayerAnimationConfig;
class UPlayerClassConfig;
class UBaseGameplayAbility;
struct FPlayerWeaponPartConfig;

/**
 *
 */
UCLASS()
class RIFTWARD_API APlayerCharacter : public ABaseCharacter
{
	GENERATED_BODY()

public:
	APlayerCharacter();

	void HandleMove(const FVector2D& InputValue);
	void ToggleWalkRun();
	bool IsRunning() const;
	// Whether Player press WASD
	bool HasMovementInput() const;
	// Whether Player is Moving
	bool IsMovementAccelerating() const;

	virtual void Tick(float DeltaTime) override;
	virtual void PostInitializeComponents() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Getter
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
	TArray<APlayerWeapon*> GetEquippedWeapons() const;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera")
	TObjectPtr<USpringArmComponent> SpringArmComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera")
	TObjectPtr<UCameraComponent> CameraComponent;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Locomotion")
	FVector2D MovementInputVector = FVector2D::ZeroVector;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Locomotion")
	bool bWantsToRun = true;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Locomotion|Turn")
	float CurrentTurnRate = 600.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, ReplicatedUsing=OnRep_PlayerClassConfig, Category="Class")
	TObjectPtr<UPlayerClassConfig> PlayerClassConfig;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Weapon")
	TArray<TObjectPtr<APlayerWeapon>> EquippedWeapons;

private:
	void InitPlayerProperties();
	void InitGasActorInfo();
	void InitCameraComponents();
	void AssemblePlayerClass();
	void ApplyAttributesFromConfig() const;
	void ApplyAnimationConfig() const;
	void ApplyWeaponsFromConfig();
	void ClearEquippedWeapons();
	APlayerWeapon* SpawnAndAttachWeapon(const FPlayerWeaponPartConfig& WeaponPartConfig);

	UFUNCTION()
	void OnRep_PlayerClassConfig();

	/* Movement and Camera Control */
	void InitMovementSettings();
	void ApplyCameraRelativeMovementInput();
	void ClearMovementInput();
	void ApplyMovementSettings() const;
	void UpdateMovementRotationRate(float DeltaTime);

	/* Gameplay Ability System */

	void GrantClassAbilities();
	void ClearClassAbilities();

	void GiveConfiguredAbility(
		UAbilitySystemComponent* ASC,
		UBaseAbilityConfig* AbilityConfig
	);

	void GiveAbilityFromClass(
		UAbilitySystemComponent* ASC,
		TSubclassOf<UBaseGameplayAbility> AbilityClass,
		UObject* SourceObject
	);

	TArray<FGameplayAbilitySpecHandle> GrantedClassAbilityHandles;
};
