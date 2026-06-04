// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Attributes/ResourceAttributeSet.h"

#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"

UResourceAttributeSet::UResourceAttributeSet()
{
	InitMaxMana(100.0f);
	InitMana(100.0f);

	InitMaxStamina(100.0f);
	InitStamina(100.0f);

	InitMaxUltimateCharge(100.0f);
	InitUltimateCharge(0.0f);

}

void UResourceAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UResourceAttributeSet, Stamina, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UResourceAttributeSet, MaxStamina, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UResourceAttributeSet, Mana, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UResourceAttributeSet, MaxMana, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UResourceAttributeSet, UltimateCharge, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UResourceAttributeSet, MaxUltimateCharge, COND_None, REPNOTIFY_Always);
}

void UResourceAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
	if (Attribute == GetManaAttribute() || Attribute == GetMaxManaAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxMana());
	} else if (Attribute == GetStaminaAttribute() || Attribute == GetMaxStaminaAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxStamina());
	} else if (Attribute == GetUltimateChargeAttribute() || Attribute == GetMaxUltimateChargeAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxUltimateCharge());
	}
}

void UResourceAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if (Data.EvaluatedData.Attribute == GetManaAttribute() || Data.EvaluatedData.Attribute == GetMaxManaAttribute())
	{
		SetMana(FMath::Clamp(GetMana(), 0.0f, GetMaxMana()));
	} else if (Data.EvaluatedData.Attribute == GetStaminaAttribute() || Data.EvaluatedData.Attribute == GetMaxStaminaAttribute())
	{
		SetStamina(FMath::Clamp(GetStamina(), 0.0f, GetMaxStamina()));
	} else if (Data.EvaluatedData.Attribute == GetUltimateChargeAttribute() || Data.EvaluatedData.Attribute == GetMaxUltimateChargeAttribute())
	{
		SetUltimateCharge(FMath::Clamp(GetUltimateCharge(), 0.0f, GetMaxUltimateCharge()));
	}
}

void UResourceAttributeSet::OnRep_Stamina(const FGameplayAttributeData& OldStamina) const
{
}

void UResourceAttributeSet::OnRep_MaxStamina(const FGameplayAttributeData& OldMaxStamina) const
{
}

void UResourceAttributeSet::OnRep_Mana(const FGameplayAttributeData& OldMana) const
{
}

void UResourceAttributeSet::OnRep_MaxMana(const FGameplayAttributeData& OldMaxMana) const
{
}

void UResourceAttributeSet::OnRep_UltimateCharge(const FGameplayAttributeData& OldUltimateCharge) const
{
}

void UResourceAttributeSet::OnRep_MaxUltimateCharge(const FGameplayAttributeData& OldMaxUltimateCharge) const
{
}
