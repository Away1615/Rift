// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_ComboAttack.generated.h"

UCLASS()
class RIFT_API UGA_ComboAttack : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_ComboAttack();

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData
	) override;

	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled
	) override;

	virtual void InputPressed(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo
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

	bool bIsFinishingAttack = false;
	int32 CurrentComboIndex = 0;
	bool bComboInputWindowOpen = false;
	bool bPendingComboInput = false;
	bool bPreBufferedComboInput = false;
	float PreBufferedInputExpireTime = 0.0f;
	float ComboInputBufferDuration = 0.25f;
	TArray<FName> ActiveComboSections;
	UPROPERTY(Transient)
	TObjectPtr<UAnimMontage> ActiveAttackMontage;
};
