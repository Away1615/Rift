// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Attributes/RiftPlayerAttributeSet.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystem/RiftGameplayTags.h"
#include "Character/PlayerCharacter.h"
#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"

URiftPlayerAttributeSet::URiftPlayerAttributeSet()
{
	InitHealth(100.0f);
	InitMaxHealth(100.0f);
	InitStamina(100.0f);
	InitMaxStamina(100.0f);
	InitPoise(100.0f);
	InitMaxPoise(100.0f);
	InitDamage(0.0f);
	InitPoiseDamage(0.0f);
}

void URiftPlayerAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(URiftPlayerAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(URiftPlayerAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(URiftPlayerAttributeSet, Stamina, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(URiftPlayerAttributeSet, MaxStamina, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(URiftPlayerAttributeSet, Poise, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(URiftPlayerAttributeSet, MaxPoise, COND_None, REPNOTIFY_Always);
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
						PlayerCharacter->HandleDeath(Data.EffectSpec.GetContext().GetInstigator());
					}
				}
			}
		}
	}

	if (Data.EvaluatedData.Attribute == GetStaminaAttribute())
	{
		SetStamina(FMath::Clamp(GetStamina(), 0.0f, GetMaxStamina()));
	}

	if (Data.EvaluatedData.Attribute == GetPoiseAttribute())
	{
		SetPoise(FMath::Clamp(GetPoise(), 0.0f, GetMaxPoise()));
	}

	if (Data.EvaluatedData.Attribute == GetPoiseDamageAttribute())
	{
		const float PoiseDamageValue = GetPoiseDamage();
		SetPoiseDamage(0.0f);
		if (PoiseDamageValue <= 0.0f) return;

		UAbilitySystemComponent* AbilitySystemComponent = GetOwningAbilitySystemComponent();
		if (AbilitySystemComponent &&
			AbilitySystemComponent->HasMatchingGameplayTag(RiftGameplayTags::State_SuperArmor_Red))
		{
			return;
		}

		float NewPoise = GetPoise() - PoiseDamageValue;
		bool bPoiseBroken = false;
		if (NewPoise <= 0.0f)
		{
			bPoiseBroken = true;
			NewPoise = GetMaxPoise();
		}

		SetPoise(FMath::Clamp(NewPoise, 0.0f, GetMaxPoise()));
		if (bPoiseBroken)
		{
			if (AbilitySystemComponent)
			{
				if (APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(AbilitySystemComponent->GetAvatarActor()))
				{
					PlayerCharacter->HandlePoiseBroken(Data.EffectSpec.GetContext().GetInstigator());
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

void URiftPlayerAttributeSet::OnRep_Stamina(const FGameplayAttributeData& OldStamina)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URiftPlayerAttributeSet, Stamina, OldStamina);
}

void URiftPlayerAttributeSet::OnRep_MaxStamina(const FGameplayAttributeData& OldMaxStamina)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URiftPlayerAttributeSet, MaxStamina, OldMaxStamina);
}

void URiftPlayerAttributeSet::OnRep_Poise(const FGameplayAttributeData& OldPoise)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URiftPlayerAttributeSet, Poise, OldPoise);
}

void URiftPlayerAttributeSet::OnRep_MaxPoise(const FGameplayAttributeData& OldMaxPoise)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URiftPlayerAttributeSet, MaxPoise, OldMaxPoise);
}
