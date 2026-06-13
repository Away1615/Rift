// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet.h"
#include "RiftEnemyAttributeSet.generated.h"

#ifndef ATTRIBUTE_ACCESSORS
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)
#endif

UCLASS()
class RIFT_API URiftEnemyAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	URiftEnemyAttributeSet();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_Health, Category="Attributes|Health")
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(URiftEnemyAttributeSet, Health)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_MaxHealth, Category="Attributes|Health")
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(URiftEnemyAttributeSet, MaxHealth)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_Poise, Category="Attributes|Poise")
	FGameplayAttributeData Poise;
	ATTRIBUTE_ACCESSORS(URiftEnemyAttributeSet, Poise)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_MaxPoise, Category="Attributes|Poise")
	FGameplayAttributeData MaxPoise;
	ATTRIBUTE_ACCESSORS(URiftEnemyAttributeSet, MaxPoise)

	UPROPERTY()
	FGameplayAttributeData Damage;
	ATTRIBUTE_ACCESSORS(URiftEnemyAttributeSet, Damage)

	UPROPERTY()
	FGameplayAttributeData PoiseDamage;
	ATTRIBUTE_ACCESSORS(URiftEnemyAttributeSet, PoiseDamage)

protected:
	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldHealth);

	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth);

	UFUNCTION()
	void OnRep_Poise(const FGameplayAttributeData& OldPoise);

	UFUNCTION()
	void OnRep_MaxPoise(const FGameplayAttributeData& OldMaxPoise);
};
