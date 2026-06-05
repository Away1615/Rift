// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Attributes/ResourceAttributeSet.h"

#include "GameplayEffectExtension.h"
#include "Debug/Logger.h"
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

	// Mana
	if (Attribute == GetManaAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxMana());
	}
	else if (Attribute == GetMaxManaAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.0f);
	}

	// Stamina
	else if (Attribute == GetStaminaAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxStamina());
	}
	else if (Attribute == GetMaxStaminaAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.0f);
	}

	// UltimateCharge
	else if (Attribute == GetUltimateChargeAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxUltimateCharge());
	}
	else if (Attribute == GetMaxUltimateChargeAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.0f);
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
	GAMEPLAYATTRIBUTE_REPNOTIFY(UResourceAttributeSet, Stamina, OldStamina);

	const AActor* Owner = GetOwningActor();

	Logger::Log(
		Owner,
		FString::Printf(
			TEXT("%s Stamina replicated: Old=%.0f New=%.0f Max=%.0f"),
			*GetNameSafe(Owner),
			OldStamina.GetCurrentValue(),
			GetStamina(),
			GetMaxStamina()
		)
	);
}

void UResourceAttributeSet::OnRep_MaxStamina(const FGameplayAttributeData& OldMaxStamina) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UResourceAttributeSet, MaxStamina, OldMaxStamina);

}

void UResourceAttributeSet::OnRep_Mana(const FGameplayAttributeData& OldMana) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UResourceAttributeSet, Mana, OldMana);
}

void UResourceAttributeSet::OnRep_MaxMana(const FGameplayAttributeData& OldMaxMana) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UResourceAttributeSet, MaxMana, OldMaxMana);
}

void UResourceAttributeSet::OnRep_UltimateCharge(const FGameplayAttributeData& OldUltimateCharge) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UResourceAttributeSet, UltimateCharge, OldUltimateCharge);
}

void UResourceAttributeSet::OnRep_MaxUltimateCharge(const FGameplayAttributeData& OldMaxUltimateCharge) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UResourceAttributeSet, MaxUltimateCharge, OldMaxUltimateCharge);
}
