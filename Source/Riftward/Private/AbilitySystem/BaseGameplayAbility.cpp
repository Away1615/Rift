// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/BaseGameplayAbility.h"

#include "GameplayTags/RiftwardGameplayTags.h"

UBaseGameplayAbility::UBaseGameplayAbility()
{
	ActivationBlockedTags.AddTag(RiftwardGameplayTags::State_Movement_Airborne);
}
