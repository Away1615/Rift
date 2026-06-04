// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseAttributeSet.h"
#include "ResourceAttributeSet.generated.h"

/**
 *
 */
UCLASS()
class RIFTWARD_API UResourceAttributeSet : public UBaseAttributeSet
{
	GENERATED_BODY()
public:
	UResourceAttributeSet();

	// Mana Points
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_Mana, Category="Attributes|Mana")
	FGameplayAttributeData Mana;
	ATTRIBUTE_ACCESSORS(UResourceAttributeSet, Mana)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_MaxMana, Category="Attributes|Mana")
	FGameplayAttributeData MaxMana;
	ATTRIBUTE_ACCESSORS(UResourceAttributeSet, MaxMana)

	// Stamina Points
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_Stamina, Category="Attributes|Stamina")
	FGameplayAttributeData Stamina;
	ATTRIBUTE_ACCESSORS(UResourceAttributeSet, Stamina)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_MaxStamina, Category="Attributes|Stamina")
	FGameplayAttributeData MaxStamina;
	ATTRIBUTE_ACCESSORS(UResourceAttributeSet, MaxStamina)

	// UltimateCharge
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_UltimateCharge, Category="Attributes|UltimateCharge")
	FGameplayAttributeData UltimateCharge;
	ATTRIBUTE_ACCESSORS(UResourceAttributeSet, UltimateCharge)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_MaxUltimateCharge, Category="Attributes|UltimateCharge")
	FGameplayAttributeData MaxUltimateCharge;
	ATTRIBUTE_ACCESSORS(UResourceAttributeSet, MaxUltimateCharge)

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

protected:
	UFUNCTION()
	void OnRep_Stamina(const FGameplayAttributeData& OldStamina) const;

	UFUNCTION()
	void OnRep_MaxStamina(const FGameplayAttributeData& OldMaxStamina) const;

	UFUNCTION()
	void OnRep_Mana(const FGameplayAttributeData& OldMana) const;

	UFUNCTION()
	void OnRep_MaxMana(const FGameplayAttributeData& OldMaxMana) const;

	UFUNCTION()
	void OnRep_UltimateCharge(const FGameplayAttributeData& OldUltimateCharge) const;

	UFUNCTION()
	void OnRep_MaxUltimateCharge(const FGameplayAttributeData& OldMaxUltimateCharge) const;
};
