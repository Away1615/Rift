// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Attributes/RiftEnemyAttributeSet.h"

#include "Character/EnemyCharacter.h"
#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"

URiftEnemyAttributeSet::URiftEnemyAttributeSet()
{
	InitMaxHealth(100.0f);
	InitHealth(100.0f);
	InitMaxPoise(100.0f);
	InitPoise(100.0f);
	InitPoiseDamage(0.0f);
	InitDamage(0.0f);

}

void URiftEnemyAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(URiftEnemyAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(URiftEnemyAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(URiftEnemyAttributeSet, Poise, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(URiftEnemyAttributeSet, MaxPoise, COND_None, REPNOTIFY_Always);
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
			if (GetHealth() <= 0.0f)
			{
				AActor* Killer = Data.EffectSpec.GetContext().GetInstigator();
				if (AEnemyCharacter* Enemy = Cast<AEnemyCharacter>(GetOwningActor()))
				{
					Enemy->HandleDeath(Killer);
				}
			}
		}
	}

	if (Data.EvaluatedData.Attribute == GetPoiseDamageAttribute())
	{
		const float PoiseDamageValue = GetPoiseDamage();
		SetPoiseDamage(0.0f);
		if (PoiseDamageValue <= 0.0f) return;

		float NewPoise = GetPoise() - PoiseDamageValue;
		bool bPoiseBroken = false;
		if (NewPoise <= 0.0f)
		{
			bPoiseBroken = true;
			NewPoise = GetMaxPoise();
		}

		SetPoise(FMath::Clamp(NewPoise, 0.0f, GetMaxPoise()));

		FVector InstigatorLocation = FVector::ZeroVector;
		if (const AActor* Instigator = Data.EffectSpec.GetContext().GetInstigator())
		{
			InstigatorLocation = Instigator->GetActorLocation();
		}

		if (AEnemyCharacter* Enemy = Cast<AEnemyCharacter>(GetOwningActor()))
		{
			Enemy->HandlePoiseHit(bPoiseBroken, InstigatorLocation);
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

void URiftEnemyAttributeSet::OnRep_Poise(const FGameplayAttributeData& OldPoise)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URiftEnemyAttributeSet, Poise, OldPoise);
}

void URiftEnemyAttributeSet::OnRep_MaxPoise(const FGameplayAttributeData& OldMaxPoise)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URiftEnemyAttributeSet, MaxPoise, OldMaxPoise);
}
