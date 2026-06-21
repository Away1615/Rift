// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Attributes/RiftEnemyAttributeSet.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystem/RiftAttributeReactionReceiver.h"
#include "AbilitySystem/RiftGameplayTags.h"
#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"

static IRiftAttributeReactionReceiver* GetAttributeReactionReceiver(
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
			if (Data.Target.HasMatchingGameplayTag(RiftGameplayTags::State_Enemy_Intro))
			{
				return;
			}

			IRiftAttributeReactionReceiver* Receiver = GetAttributeReactionReceiver(Data);
			const AActor* TargetActor = Data.Target.GetAvatarActor();
			const FVector DamageNumberLocation = TargetActor
				? TargetActor->GetActorLocation() + FVector(0.0f, 0.0f, 120.0f)
				: FVector::ZeroVector;

			if (Data.Target.HasMatchingGameplayTag(RiftGameplayTags::State_Blocking))
			{
				if (Receiver)
				{
					Receiver->HandleAttributeDamageNumber(0.0f, true, DamageNumberLocation);
				}
				return;
			}

			const float OldHealth = GetHealth();
			const float NewHealth = FMath::Clamp(OldHealth - DamageValue, 0.0f, GetMaxHealth());
			const float ActualDamage = FMath::Max(0.0f, OldHealth - NewHealth);
			SetHealth(NewHealth);

			if (ActualDamage > 0.0f && Receiver)
			{
				Receiver->HandleAttributeDamageNumber(ActualDamage, false, DamageNumberLocation);
			}

			if (GetHealth() <= 0.0f && Receiver)
			{
				Receiver->HandleAttributeDeath(Data.EffectSpec.GetContext().GetInstigator());
			}
		}
	}

	if (Data.EvaluatedData.Attribute == GetPoiseDamageAttribute())
	{
		float PoiseDamageValue = GetPoiseDamage();
		SetPoiseDamage(0.0f);

		if (Data.Target.HasMatchingGameplayTag(RiftGameplayTags::State_Enemy_Intro))
		{
			return;
		}

		IRiftAttributeReactionReceiver* Receiver = GetAttributeReactionReceiver(Data);
		if (Data.Target.HasMatchingGameplayTag(RiftGameplayTags::State_Blocking))
		{
			const float BlockingPoiseDamageMultiplier = Receiver
				? FMath::Clamp(Receiver->GetAttributeBlockingPoiseDamageMultiplier(), 0.0f, 1.0f)
				: 1.0f;
			PoiseDamageValue *= BlockingPoiseDamageMultiplier;
		}

		if (PoiseDamageValue <= 0.0f) return;

		float NewPoise = GetPoise() - PoiseDamageValue;
		bool bPoiseBroken = false;
		if (NewPoise <= 0.0f)
		{
			bPoiseBroken = true;
			NewPoise = GetMaxPoise();
		}

		SetPoise(FMath::Clamp(NewPoise, 0.0f, GetMaxPoise()));

		if (Receiver)
		{
			Receiver->HandleAttributePoiseHit(bPoiseBroken, Data.EffectSpec.GetContext().GetInstigator());
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
