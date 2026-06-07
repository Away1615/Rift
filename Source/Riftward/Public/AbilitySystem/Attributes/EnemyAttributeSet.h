// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Attributes/BaseAttributeSet.h"
#include "EnemyAttributeSet.generated.h"

UCLASS()
class RIFTWARD_API UEnemyAttributeSet : public UBaseAttributeSet
{
	GENERATED_BODY()

public:
	UEnemyAttributeSet();

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_Poise, Category="Attributes|Poise")
	FGameplayAttributeData Poise;
	ATTRIBUTE_ACCESSORS(UEnemyAttributeSet, Poise)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_MaxPoise, Category="Attributes|Poise")
	FGameplayAttributeData MaxPoise;
	ATTRIBUTE_ACCESSORS(UEnemyAttributeSet, MaxPoise)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_Rage, Category="Attributes|Rage")
	FGameplayAttributeData Rage;
	ATTRIBUTE_ACCESSORS(UEnemyAttributeSet, Rage)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_MaxRage, Category="Attributes|Rage")
	FGameplayAttributeData MaxRage;
	ATTRIBUTE_ACCESSORS(UEnemyAttributeSet, MaxRage)

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

protected:
	UFUNCTION()
	void OnRep_Poise(const FGameplayAttributeData& OldPoise) const;

	UFUNCTION()
	void OnRep_MaxPoise(const FGameplayAttributeData& OldMaxPoise) const;

	UFUNCTION()
	void OnRep_Rage(const FGameplayAttributeData& OldRage) const;

	UFUNCTION()
	void OnRep_MaxRage(const FGameplayAttributeData& OldMaxRage) const;
};
