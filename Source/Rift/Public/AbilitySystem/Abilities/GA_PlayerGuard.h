#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_PlayerGuard.generated.h"

class UAbilitySystemComponent;
class UAnimMontage;

UCLASS()
class RIFT_API UGA_PlayerGuard : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_PlayerGuard();

	virtual void ActivateAbility(
		FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData
	) override;

	virtual void InputReleased(
		FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		FGameplayAbilityActivationInfo ActivationInfo
	) override;

	virtual void EndAbility(
		FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled
	) override;

private:
	static void SetBlockingTag(UAbilitySystemComponent* AbilitySystemComponent, bool bBlocking);
	void ConfigureGuardMontageSections();
	void ConfigureGuardMontageEndSection();
	void JumpToGuardSection(FName SectionName);
	void FinishGuardAbility(bool bWasCancelled);

	UFUNCTION()
	void HandleMontageCompleted();

	UFUNCTION()
	void HandleMontageBlendOut();

	UFUNCTION()
	void HandleMontageInterrupted();

	UFUNCTION()
	void HandleMontageCancelled();

	TWeakObjectPtr<UAnimMontage> ActiveGuardMontage;
	FName ActiveGuardStartSection = NAME_None;
	FName ActiveGuardLoopSection = NAME_None;
	FName ActiveGuardEndSection = NAME_None;
	bool bGuardEnding = false;
	bool bIsFinishingGuard = false;
};
