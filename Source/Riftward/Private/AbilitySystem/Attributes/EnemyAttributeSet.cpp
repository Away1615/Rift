// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Attributes/EnemyAttributeSet.h"

#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"

UEnemyAttributeSet::UEnemyAttributeSet()
{
	InitMaxPoise(100.0f);
	InitPoise(100.0f);
	InitMaxRage(100.0f);
	InitRage(0.0f);
}

void UEnemyAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UEnemyAttributeSet, Poise, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UEnemyAttributeSet, MaxPoise, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UEnemyAttributeSet, Rage, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UEnemyAttributeSet, MaxRage, COND_None, REPNOTIFY_Always);
}

void UEnemyAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetPoiseAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxPoise());
	}
	else if (Attribute == GetMaxPoiseAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.0f);
	}
	else if (Attribute == GetRageAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxRage());
	}
	else if (Attribute == GetMaxRageAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.0f);
	}
}

void UEnemyAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if (Data.EvaluatedData.Attribute == GetPoiseAttribute()
		|| Data.EvaluatedData.Attribute == GetMaxPoiseAttribute())
	{
		SetPoise(FMath::Clamp(GetPoise(), 0.0f, GetMaxPoise()));
	}
	else if (Data.EvaluatedData.Attribute == GetRageAttribute()
		|| Data.EvaluatedData.Attribute == GetMaxRageAttribute())
	{
		SetRage(FMath::Clamp(GetRage(), 0.0f, GetMaxRage()));
	}
}

void UEnemyAttributeSet::OnRep_Poise(const FGameplayAttributeData& OldPoise) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UEnemyAttributeSet, Poise, OldPoise);
}

void UEnemyAttributeSet::OnRep_MaxPoise(const FGameplayAttributeData& OldMaxPoise) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UEnemyAttributeSet, MaxPoise, OldMaxPoise);
}

void UEnemyAttributeSet::OnRep_Rage(const FGameplayAttributeData& OldRage) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UEnemyAttributeSet, Rage, OldRage);
}

void UEnemyAttributeSet::OnRep_MaxRage(const FGameplayAttributeData& OldMaxRage) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UEnemyAttributeSet, MaxRage, OldMaxRage);
}
