// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/RiftAttributeSet.h"

#include "Net/UnrealNetwork.h"

URiftAttributeSet::URiftAttributeSet()
{
	InitHealth(100.0f);
	InitMaxHealth(100.0f);
}

void URiftAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(URiftAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(URiftAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
}

void URiftAttributeSet::OnRep_Health(const FGameplayAttributeData& OldHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URiftAttributeSet, Health, OldHealth);
}

void URiftAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URiftAttributeSet, MaxHealth, OldMaxHealth);
}
