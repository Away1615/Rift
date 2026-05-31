// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/BaseGameplayAbility.h"
#include "GameplayEffectTypes.h"
#include "GA_Player_Sprint.generated.h"

class UGameplayEffect;
struct FOnAttributeChangeData;

UCLASS()
class RIFTWARD_API UGA_Player_Sprint : public UBaseGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_Player_Sprint();

	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Cost")
	TSubclassOf<UGameplayEffect> SprintCostEffectClass;

private:
	FActiveGameplayEffectHandle SprintCostEffectHandle;
	FDelegateHandle StaminaChangedDelegateHandle;

	void HandleStaminaChanged(const FOnAttributeChangeData& Data);
};
