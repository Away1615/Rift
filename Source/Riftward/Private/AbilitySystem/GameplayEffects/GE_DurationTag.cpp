// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/GameplayEffects/GE_DurationTag.h"

#include "GameplayTags/RiftGameplayTags.h"

UGE_DurationTag::UGE_DurationTag()
{
	DurationPolicy = EGameplayEffectDurationType::HasDuration;

	FSetByCallerFloat SetByCallerDuration;
	SetByCallerDuration.DataTag = FRiftGameplayTags::Get().Data_Duration;
	DurationMagnitude = FGameplayEffectModifierMagnitude(SetByCallerDuration);
}
