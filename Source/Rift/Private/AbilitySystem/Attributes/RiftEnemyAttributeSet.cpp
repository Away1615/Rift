// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Attributes/RiftEnemyAttributeSet.h"

#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"

URiftEnemyAttributeSet::URiftEnemyAttributeSet()
{
	InitHealth(100.0f);
	InitMaxHealth(100.0f);
	InitDamage(0.0f);
}

void URiftEnemyAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(URiftEnemyAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(URiftEnemyAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
}

void URiftEnemyAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if (Data.EvaluatedData.Attribute == GetDamageAttribute())
	{
		const float DamageValue = GetDamage();
		SetDamage(0.0f);

		if (DamageValue > 0.0f)
		{
			SetHealth(FMath::Clamp(GetHealth() - DamageValue, 0.0f, GetMaxHealth()));
		}
	}
}

void URiftEnemyAttributeSet::OnRep_Health(const FGameplayAttributeData& OldHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URiftEnemyAttributeSet, Health, OldHealth);
}

void URiftEnemyAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URiftEnemyAttributeSet, MaxHealth, OldMaxHealth);
}
