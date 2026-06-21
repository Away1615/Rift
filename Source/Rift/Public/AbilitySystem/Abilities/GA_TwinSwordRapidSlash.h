#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "Combat/RiftCombatFeedbackTypes.h"
#include "GameplayTagContainer.h"
#include "GA_TwinSwordRapidSlash.generated.h"

class APlayerCharacter;
class UAnimMontage;
class UCameraShakeBase;
class UNiagaraSystem;
class URiftCombatCueConfig;
class UTwinSwordRapidSlashAbilityConfig;

struct FRiftResolvedTwinSwordRapidSlashConfig
{
	UAnimMontage* Montage = nullptr;
	FName SectionA = TEXT("Rapid_1");
	FName SectionB = TEXT("Rapid_2");
	FName FinisherSection = TEXT("Rapid_Finisher");
	float PlayRate = 1.35f;
	float AssistFacingDuration = 0.12f;
	float AssistFacingRotationSpeed = 1440.0f;
	float Damage = 8.0f;
	float PoiseDamage = 5.0f;
	float UltimateChargeOnHit = 3.0f;
	float FinisherDamage = 28.0f;
	float FinisherPoiseDamage = 45.0f;
	float FinisherUltimateChargeOnHit = 6.0f;
	FRiftMeleeHitStopConfig HitStopConfig;
	URiftCombatCueConfig* CombatCueConfig = nullptr;
	TSubclassOf<UCameraShakeBase> CameraShake;
	FVector2D CameraShakeDir = FVector2D(1.0f, 0.0f);
	FRiftMeleeHitStopConfig FinisherHitStopConfig;
	URiftCombatCueConfig* FinisherCombatCueConfig = nullptr;
	TSubclassOf<UCameraShakeBase> FinisherCameraShake;
	FVector2D FinisherCameraShakeDir = FVector2D(1.0f, 0.0f);
	UNiagaraSystem* RapidSlashAuraNiagara = nullptr;
	FName RapidSlashAuraAttachSocketName = NAME_None;
	FVector RapidSlashAuraLocationOffset = FVector::ZeroVector;
	FRotator RapidSlashAuraRotationOffset = FRotator::ZeroRotator;
	FVector RapidSlashAuraScale = FVector(1.0f);
};

UCLASS()
class RIFT_API UGA_TwinSwordRapidSlash : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_TwinSwordRapidSlash();

	virtual bool CanActivateAbility(
		FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayTagContainer* SourceTags = nullptr,
		const FGameplayTagContainer* TargetTags = nullptr,
		FGameplayTagContainer* OptionalRelevantTags = nullptr
	) const override;

	virtual void ActivateAbility(
		FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData
	) override;

	virtual void EndAbility(
		FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled
	) override;

	virtual void InputPressed(
		FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		FGameplayAbilityActivationInfo ActivationInfo
	) override;

	void OpenComboChainWindow();
	void CloseComboChainWindow();
	void RequestFinisherFromInput();

	static UGA_TwinSwordRapidSlash* FindActiveRapidSlashInstance(AActor* AvatarActor);

protected:
	UFUNCTION()
	void HandleMontageCompleted();

	UFUNCTION()
	void HandleMontageBlendOut();

	UFUNCTION()
	void HandleMontageInterrupted();

	UFUNCTION()
	void HandleMontageCancelled();

private:
	bool ResolveRapidSlashContext(
		APlayerCharacter*& OutPlayerCharacter,
		FRiftResolvedTwinSwordRapidSlashConfig& OutConfig
	) const;
	void FillRapidSlashConfigFromAbilityConfig(
		const UTwinSwordRapidSlashAbilityConfig* AbilityConfig,
		FRiftResolvedTwinSwordRapidSlashConfig& OutConfig
	) const;
	void ClearRapidSlashState();
	void FinishRapidSlashAbility(bool bWasCancelled);
	void SetRapidSlashState(bool bReady);
	void SetRapidSlashReadyState(bool bReady);
	void SetRapidSlashWeaponTrace(bool bUseFinisherParams);
	void ApplyTargetAssistFacing();
	bool TryCommitRapidSlashChain();
	void JumpToRapidSection(FName SectionName);
	FName ResolveNextLoopSection(const FRiftResolvedTwinSwordRapidSlashConfig& Config) const;

	FName CurrentRapidSection = NAME_None;
	bool bPendingContinueInput = false;
	bool bPendingFinisherInput = false;
	bool bIsFinisherPlaying = false;
	bool bComboChainWindowOpen = false;
	bool bIsFinishingRapidSlash = false;

	UPROPERTY(Transient)
	TObjectPtr<UAnimMontage> ActiveRapidSlashMontage;
};
