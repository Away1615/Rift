// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/BaseAbilitySystemComponent.h"

#include "GameplayEffectTypes.h"
#include "GameplayTags/RiftGameplayTags.h"

bool UBaseAbilitySystemComponent::TryBufferAttackInputForActiveCombo(const EAbilityInputID NewInputID)
{
	if (!BufferAttackInputForActiveCombo(NewInputID))
	{
		return false;
	}

	if (!IsOwnerActorAuthoritative())
	{
		ServerBufferAttackInputForActiveCombo(NewInputID);
	}

	return true;
}

void UBaseAbilitySystemComponent::ServerBufferAttackInputForActiveCombo_Implementation(const EAbilityInputID NewInputID)
{
	BufferAttackInputForActiveCombo(NewInputID);
}

bool UBaseAbilitySystemComponent::BufferAttackInputForActiveCombo(const EAbilityInputID NewInputID)
{
	const FRiftGameplayTags& RiftTags = FRiftGameplayTags::Get();

	EAbilityInputID OppositeInputID = EAbilityInputID::None;
	FGameplayTag InputEventTag;
	switch (NewInputID)
	{
	case EAbilityInputID::Primary:
		OppositeInputID = EAbilityInputID::Secondary;
		InputEventTag = RiftTags.Event_TwinSword_Combo_Input_Primary;
		break;
	case EAbilityInputID::Secondary:
		OppositeInputID = EAbilityInputID::Primary;
		InputEventTag = RiftTags.Event_TwinSword_Combo_Input_Secondary;
		break;
	default:
		return false;
	}

	if (!HasActiveAbilityForInput(OppositeInputID))
	{
		return false;
	}

	FGameplayEventData Payload;
	Payload.EventTag = InputEventTag;
	Payload.Instigator = GetAvatarActor();
	Payload.Target = GetAvatarActor();
	HandleGameplayEvent(InputEventTag, &Payload);
	return true;
}

void UBaseAbilitySystemComponent::CancelActiveAbilitiesInterruptedByInput(const EAbilityInputID NewInputID)
{
	TArray<int32> InputIDsToCancel;

	if (NewInputID == EAbilityInputID::Core)
	{
		// 翻滚打断所有连招与重击（不应打断 Q/E/Shift 大招释放，那些通过 BlockedTags 阻止 Dodge 激活）
		InputIDsToCancel.Add(static_cast<int32>(EAbilityInputID::Primary));
		InputIDsToCancel.Add(static_cast<int32>(EAbilityInputID::Secondary));
		InputIDsToCancel.Add(static_cast<int32>(EAbilityInputID::PrimaryHeavy));
		InputIDsToCancel.Add(static_cast<int32>(EAbilityInputID::SecondaryHeavy));
	}
	else if (NewInputID == EAbilityInputID::PrimaryHeavy)
	{
		// 长按重击打断进行中的普通连招
		InputIDsToCancel.Add(static_cast<int32>(EAbilityInputID::Primary));
	}
	else if (NewInputID == EAbilityInputID::SecondaryHeavy)
	{
		// 长按右键强化重击打断进行中的右键连招
		InputIDsToCancel.Add(static_cast<int32>(EAbilityInputID::Secondary));
	}

	if (InputIDsToCancel.IsEmpty())
	{
		return;
	}

	TArray<FGameplayAbilitySpecHandle> HandlesToCancel;
	{
		ABILITYLIST_SCOPE_LOCK();
		for (const FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
		{
			if (InputIDsToCancel.Contains(AbilitySpec.InputID) && AbilitySpec.IsActive())
			{
				HandlesToCancel.Add(AbilitySpec.Handle);
			}
		}
	}

	for (const FGameplayAbilitySpecHandle& Handle : HandlesToCancel)
	{
		CancelAbilityHandle(Handle);
	}
}

bool UBaseAbilitySystemComponent::HasActiveAbilityForInput(const EAbilityInputID InputID)
{
	const int32 InputIDValue = static_cast<int32>(InputID);

	ABILITYLIST_SCOPE_LOCK();
	for (const FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if (AbilitySpec.InputID == InputIDValue && AbilitySpec.IsActive())
		{
			return true;
		}
	}

	return false;
}
