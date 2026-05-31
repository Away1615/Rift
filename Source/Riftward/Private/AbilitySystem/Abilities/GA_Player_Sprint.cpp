// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Abilities/GA_Player_Sprint.h"

#include "AbilitySystem/BaseAttributeSet.h"
#include "AbilitySystem/Effects/GE_Cost_Stamina_Sprint.h"
#include "AbilitySystemComponent.h"
#include "Character/PlayerCharacter.h"
#include "GameplayTags/RiftGameplayTags.h"

UGA_Player_Sprint::UGA_Player_Sprint()
{
	AbilityInputID = EAbilityInputID::Sprint;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	SprintCostEffectClass = UGE_Cost_Stamina_Sprint::StaticClass();
}

bool UGA_Player_Sprint::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                           const FGameplayAbilityActorInfo* ActorInfo,
                                           const FGameplayTagContainer* SourceTags,
                                           const FGameplayTagContainer* TargetTags,
                                           FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	const UAbilitySystemComponent* ASC = ActorInfo ? ActorInfo->AbilitySystemComponent.Get() : nullptr;
	return ASC && ASC->GetNumericAttribute(UBaseAttributeSet::GetSPAttribute()) > 0.0f;
}

void UGA_Player_Sprint::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                        const FGameplayAbilityActorInfo* ActorInfo,
                                        const FGameplayAbilityActivationInfo ActivationInfo,
                                        const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	APlayerCharacter* Character = Cast<APlayerCharacter>(GetAvatarActorFromActorInfo());
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!Character || !ASC)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	Character->SetSprinting(true);
	ASC->SetLooseGameplayTagCount(FRiftGameplayTags::Get().State_Action_Sprinting, 1);

	if (SprintCostEffectClass)
	{
		const UGameplayEffect* SprintCostEffect = SprintCostEffectClass->GetDefaultObject<UGameplayEffect>();
		SprintCostEffectHandle = ApplyGameplayEffectToOwner(Handle, ActorInfo, ActivationInfo, SprintCostEffect, GetAbilityLevel(Handle, ActorInfo));
	}

	StaminaChangedDelegateHandle = ASC->GetGameplayAttributeValueChangeDelegate(UBaseAttributeSet::GetSPAttribute())
		.AddUObject(this, &UGA_Player_Sprint::HandleStaminaChanged);
}

void UGA_Player_Sprint::InputReleased(const FGameplayAbilitySpecHandle Handle,
                                      const FGameplayAbilityActorInfo* ActorInfo,
                                      const FGameplayAbilityActivationInfo ActivationInfo)
{
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UGA_Player_Sprint::EndAbility(const FGameplayAbilitySpecHandle Handle,
                                   const FGameplayAbilityActorInfo* ActorInfo,
                                   const FGameplayAbilityActivationInfo ActivationInfo,
                                   bool bReplicateEndAbility,
                                   bool bWasCancelled)
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (ASC)
	{
		if (SprintCostEffectHandle.IsValid())
		{
			ASC->RemoveActiveGameplayEffect(SprintCostEffectHandle);
			SprintCostEffectHandle.Invalidate();
		}

		if (StaminaChangedDelegateHandle.IsValid())
		{
			ASC->GetGameplayAttributeValueChangeDelegate(UBaseAttributeSet::GetSPAttribute())
				.Remove(StaminaChangedDelegateHandle);
			StaminaChangedDelegateHandle.Reset();
		}

		ASC->SetLooseGameplayTagCount(FRiftGameplayTags::Get().State_Action_Sprinting, 0);
	}

	if (APlayerCharacter* Character = Cast<APlayerCharacter>(GetAvatarActorFromActorInfo()))
	{
		Character->SetSprinting(false);
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_Player_Sprint::HandleStaminaChanged(const FOnAttributeChangeData& Data)
{
	if (Data.NewValue <= 0.0f && IsActive())
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	}
}
