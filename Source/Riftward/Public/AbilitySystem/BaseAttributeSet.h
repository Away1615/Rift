// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "BaseAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/**
 *
 */
UCLASS()
class RIFTWARD_API UBaseAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UBaseAttributeSet();

	// Health Points
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_HP, Category="Attributes")
	FGameplayAttributeData HP;
	ATTRIBUTE_ACCESSORS(UBaseAttributeSet, HP)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_MaxHP, Category="Attributes")
	FGameplayAttributeData MaxHP;
	ATTRIBUTE_ACCESSORS(UBaseAttributeSet, MaxHP)

	// Mana Points
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_MP, Category="Attributes")
	FGameplayAttributeData MP;
	ATTRIBUTE_ACCESSORS(UBaseAttributeSet, MP)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_MaxMP, Category="Attributes")
	FGameplayAttributeData MaxMP;
	ATTRIBUTE_ACCESSORS(UBaseAttributeSet, MaxMP)

	// Stamina Points
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_SP, Category="Attributes")
	FGameplayAttributeData SP;
	ATTRIBUTE_ACCESSORS(UBaseAttributeSet, SP)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_MaxSP, Category="Attributes")
	FGameplayAttributeData MaxSP;
	ATTRIBUTE_ACCESSORS(UBaseAttributeSet, MaxSP)

	// For Network Replicate
	// Which attributes in this AttributeSet need to be synchronized from the server to the client
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Hook before attribute change
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

	// Hook after gameplayEffect execute
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

protected:
	/*
	 * CallBacks after props replicate
	*/

	UFUNCTION()
	void OnRep_HP(const FGameplayAttributeData& OldHP) const;

	UFUNCTION()
	void OnRep_MaxHP(const FGameplayAttributeData& OldMaxHP) const;

	UFUNCTION()
	void OnRep_SP(const FGameplayAttributeData& OldSP) const;

	UFUNCTION()
	void OnRep_MaxSP(const FGameplayAttributeData& OldMaxSP) const;

	UFUNCTION()
	void OnRep_MP(const FGameplayAttributeData& OldMP) const;

	UFUNCTION()
	void OnRep_MaxMP(const FGameplayAttributeData& OldMaxMP) const;
};
