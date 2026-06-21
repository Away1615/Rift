#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GameplayTagContainer.h"
#include "TimerManager.h"
#include "GA_TwinSwordComboAttack.generated.h"

class UAnimMontage;
class URiftComboGraph;
class UTwinSwordComboAbilityConfig;
struct FRiftComboNode;

UCLASS()
class RIFT_API UGA_TwinSwordComboAttack : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_TwinSwordComboAttack();

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

	void OpenComboInputWindow();
	void CloseComboInputWindow();
	void OpenComboChainWindow();
	void CloseComboChainWindow();
	bool TryCommitComboChain();

	static UGA_TwinSwordComboAttack* FindActiveTwinSwordComboInstance(AActor* AvatarActor);

protected:
	UFUNCTION()
	void HandleMontageCompleted();

	UFUNCTION()
	void HandleMontageBlendOut();

	UFUNCTION()
	void HandleMontageInterrupted();

	UFUNCTION()
	void HandleMontageCancelled();

	void FinishAttackAbility(bool bWasCancelled);

private:
	void ClearComboState();
	void ApplyTargetAssistFacing();
	void UpdateWeaponTraceDamage();
	void EnterNode();
	bool TryStartRapidSlashFromReadyGrantSection(const FGameplayTag& InputTag);
	bool GrantRapidSlashReadyFromConfig();
	void SetRapidSlashReadyState(bool bReady);
	void ClearRapidSlashReadyState();
	const FRiftComboNode* GetCurrentNode() const;
	FName ResolveNextSection(const FRiftComboNode& CurrentNode, const FGameplayTag& InputTag) const;
	float GetAssistFacingDuration() const;
	float GetAssistFacingRotationSpeed() const;
	FName GetRapidSlashReadyGrantSection() const;
	float GetRapidSlashReadyDuration() const;

	bool bIsFinishingAttack = false;
	FName CurrentSection = NAME_None;
	FGameplayTag PendingInputTag;
	FGameplayTag PreBufferedInputTag;
	float PreBufferedInputExpireTime = 0.0f;
	bool bComboInputWindowOpen = false;
	bool bComboChainWindowOpen = false;
	bool bPendingRapidSlashChain = false;
	bool bRapidSlashChainCommitted = false;
	float ComboInputBufferDuration = 0.25f;
	FTimerHandle RapidSlashReadyTimerHandle;

	UPROPERTY(Transient)
	TObjectPtr<UAnimMontage> ActiveAttackMontage;

	UPROPERTY(Transient)
	TObjectPtr<URiftComboGraph> ActiveComboGraph;

	UPROPERTY(Transient)
	TObjectPtr<UTwinSwordComboAbilityConfig> ActiveComboConfig;
};
