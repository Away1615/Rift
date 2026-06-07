// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Attributes/HealthAttributeSet.h"

#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"
#include "GameplayTags/RiftGameplayTags.h"

UHealthAttributeSet::UHealthAttributeSet()
{
	InitMaxHealth(100.0f);
	InitHealth(100.0f);
}

void UHealthAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UHealthAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UHealthAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
}

void UHealthAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetHealthAttribute() || Attribute == GetMaxHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHealth());
	}
}

void UHealthAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		const float Magnitude = Data.EvaluatedData.Magnitude;

		// 闪避无敌帧：闪避激活期间受到的伤害全部取消，并触发完美闪避事件（由 GA_TwinSword_Dodge 监听）
		if (Magnitude < 0.0f &&
			Data.Target.HasMatchingGameplayTag(FRiftGameplayTags::Get().State_TwinSword_Dodge_Active))
		{
			SetHealth(GetHealth() - Magnitude); // 还原伤害（Magnitude 为负，减负 = 加回）
			FGameplayEventData EventPayload;
			EventPayload.EventTag = FRiftGameplayTags::Get().Event_TwinSword_Dodge_PerfectSuccess;
			Data.Target.HandleGameplayEvent(EventPayload.EventTag, &EventPayload);
			return;
		}

		SetHealth(FMath::Clamp(GetHealth(), 0.0f, GetMaxHealth()));
	}
	else if (Data.EvaluatedData.Attribute == GetMaxHealthAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth(), 0.0f, GetMaxHealth()));
	}
}

void UHealthAttributeSet::OnRep_Health(const FGameplayAttributeData& OldHealth) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHealthAttributeSet, Health, OldHealth);
}

void UHealthAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHealthAttributeSet, MaxHealth, OldMaxHealth);
}
