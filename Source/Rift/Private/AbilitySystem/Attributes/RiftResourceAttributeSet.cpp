#include "AbilitySystem/Attributes/RiftResourceAttributeSet.h"

#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"

URiftResourceAttributeSet::URiftResourceAttributeSet()
{
	InitUltimateCharge(0.0f);
	InitMaxUltimateCharge(100.0f);
}

void URiftResourceAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(URiftResourceAttributeSet, UltimateCharge, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(URiftResourceAttributeSet, MaxUltimateCharge, COND_None, REPNOTIFY_Always);
}

void URiftResourceAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if (Data.EvaluatedData.Attribute == GetUltimateChargeAttribute())
	{
		SetUltimateCharge(FMath::Clamp(GetUltimateCharge(), 0.0f, GetMaxUltimateCharge()));
	}
}

void URiftResourceAttributeSet::OnRep_UltimateCharge(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URiftResourceAttributeSet, UltimateCharge, OldValue);
}

void URiftResourceAttributeSet::OnRep_MaxUltimateCharge(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URiftResourceAttributeSet, MaxUltimateCharge, OldValue);
}
