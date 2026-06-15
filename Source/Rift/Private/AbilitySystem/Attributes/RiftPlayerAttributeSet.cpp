// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Attributes/RiftPlayerAttributeSet.h"

#include "AbilitySystemComponent.h"
#include "Character/PlayerCharacter.h"
#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"

URiftPlayerAttributeSet::URiftPlayerAttributeSet()
{
	InitHealth(100.0f);
	InitMaxHealth(100.0f);
	InitStamina(100.0f);
	InitMaxStamina(100.0f);
	InitDamage(0.0f);
}

void URiftPlayerAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(URiftPlayerAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(URiftPlayerAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(URiftPlayerAttributeSet, Stamina, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(URiftPlayerAttributeSet, MaxStamina, COND_None, REPNOTIFY_Always);
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
				if (UAbilitySystemComponent* AbilitySystemComponent = GetOwningAbilitySystemComponent())
				{
					if (APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(AbilitySystemComponent->GetAvatarActor()))
					{
						PlayerCharacter->HandleDeath();
					}
				}
			}
		}
	}

	if (Data.EvaluatedData.Attribute == GetStaminaAttribute())
	{
		SetStamina(FMath::Clamp(GetStamina(), 0.0f, GetMaxStamina()));
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

void URiftPlayerAttributeSet::OnRep_Stamina(const FGameplayAttributeData& OldStamina)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URiftPlayerAttributeSet, Stamina, OldStamina);
}

void URiftPlayerAttributeSet::OnRep_MaxStamina(const FGameplayAttributeData& OldMaxStamina)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URiftPlayerAttributeSet, MaxStamina, OldMaxStamina);
}
