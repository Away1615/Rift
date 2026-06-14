#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GameplayTagContainer.h"
#include "GA_ComboAttack.generated.h"

class UAnimMontage;
class URiftComboGraph;
struct FRiftComboNode;

UCLASS()
class RIFT_API UGA_ComboAttack : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_ComboAttack();

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
	void CommitComboChainPoint();

	static UGA_ComboAttack* FindActiveComboInstance(AActor* AvatarActor);

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
	bool TryChargeStamina(float Cost);
	const FRiftComboNode* GetCurrentNode() const;

	bool bIsFinishingAttack = false;
	FName CurrentSection = NAME_None;
	FGameplayTag PendingInputTag;
	FGameplayTag PreBufferedInputTag;
	float PreBufferedInputExpireTime = 0.0f;
	bool bComboInputWindowOpen = false;
	float ComboInputBufferDuration = 0.25f;

	UPROPERTY(Transient)
	TObjectPtr<UAnimMontage> ActiveAttackMontage;

	UPROPERTY(Transient)
	TObjectPtr<URiftComboGraph> ActiveComboGraph;
};
