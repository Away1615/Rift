// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Attributes/RiftPlayerAttributeSet.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystem/RiftAttributeReactionReceiver.h"
#include "AbilitySystem/RiftGameplayTags.h"
#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"

static IRiftAttributeReactionReceiver* GetPlayerAttributeReactionReceiver(
	const FGameplayEffectModCallbackData& Data
)
{
	if (AActor* AvatarActor = Data.Target.GetAvatarActor())
	{
		if (IRiftAttributeReactionReceiver* Receiver = Cast<IRiftAttributeReactionReceiver>(AvatarActor))
		{
			return Receiver;
		}
	}

	if (AActor* OwnerActor = Data.Target.GetOwnerActor())
	{
		return Cast<IRiftAttributeReactionReceiver>(OwnerActor);
	}

	return nullptr;
}

URiftPlayerAttributeSet::URiftPlayerAttributeSet()
{
	InitHealth(100.0f);
	InitMaxHealth(100.0f);
	InitDamage(0.0f);
}

void URiftPlayerAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(URiftPlayerAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(URiftPlayerAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
}

void URiftPlayerAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth(), 0.0f, GetMaxHealth()));
	}

	if (Data.EvaluatedData.Attribute == GetDamageAttribute())
	{
		const float DamageValue = GetDamage();
		SetDamage(0.0f);
		if (DamageValue > 0.0f)
		{
			SetHealth(FMath::Clamp(GetHealth() - DamageValue, 0.0f, GetMaxHealth()));
			if (GetHealth() <= 0.0f)
			{
				if (IRiftAttributeReactionReceiver* Receiver = GetPlayerAttributeReactionReceiver(Data))
				{
					Receiver->HandleAttributeDeath(Data.EffectSpec.GetContext().GetInstigator());
				}
			}
		}
	}

}

void URiftPlayerAttributeSet::OnRep_Health(const FGameplayAttributeData& OldHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URiftPlayerAttributeSet, Health, OldHealth);
}

void URiftPlayerAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URiftPlayerAttributeSet, MaxHealth, OldMaxHealth);
}
