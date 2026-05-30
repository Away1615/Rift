// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/BaseAttributeSet.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"


UBaseAttributeSet::UBaseAttributeSet()
{
	InitMaxHP(100.0f);
	InitHP(100.0f);

	InitMaxMP(100.0f);
	InitMP(100.0f);

	InitMaxSP(100.0f);
	InitSP(100.0f);

}

void UBaseAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UBaseAttributeSet, HP, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UBaseAttributeSet, MaxHP, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UBaseAttributeSet, MP, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UBaseAttributeSet, MaxMP, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UBaseAttributeSet, SP, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UBaseAttributeSet, MaxSP, COND_None, REPNOTIFY_Always);
}

void UBaseAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetHPAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHP());
	} else if (Attribute == GetMPAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxMP());
	} else if (Attribute == GetSPAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxSP());
	}
}

void UBaseAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if (Data.EvaluatedData.Attribute == GetHPAttribute())
	{
		SetHP(FMath::Clamp(GetHP(), 0.0f, GetMaxHP()));
	} else if (Data.EvaluatedData.Attribute == GetMPAttribute())
	{
		SetMP(FMath::Clamp(GetMP(), 0.0f, GetMaxMP()));
	} else if (Data.EvaluatedData.Attribute == GetSPAttribute())
	{
		SetSP(FMath::Clamp(GetSP(), 0.0f, GetMaxSP()));
	}

}

// Tell AbilitySystemComponent Gameplay Attribute: HP is updating by server and old value is OldHP
// component need to notify this prop has changed
void UBaseAttributeSet::OnRep_HP(const FGameplayAttributeData& OldHP) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UBaseAttributeSet, HP, OldHP);
}

void UBaseAttributeSet::OnRep_MaxHP(const FGameplayAttributeData& OldMaxHP) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UBaseAttributeSet, MaxHP, OldMaxHP);
}

void UBaseAttributeSet::OnRep_MP(const FGameplayAttributeData& OldMP) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UBaseAttributeSet, MP, OldMP);
}

void UBaseAttributeSet::OnRep_MaxMP(const FGameplayAttributeData& OldMaxMP) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UBaseAttributeSet, MaxMP, OldMaxMP);
}

void UBaseAttributeSet::OnRep_SP(const FGameplayAttributeData& OldSP) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UBaseAttributeSet, SP, OldSP);
}

void UBaseAttributeSet::OnRep_MaxSP(const FGameplayAttributeData& OldMaxSP) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UBaseAttributeSet, MaxSP, OldMaxSP);
}
