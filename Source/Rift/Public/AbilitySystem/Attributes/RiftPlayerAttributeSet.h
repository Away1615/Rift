// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Attributes/RiftAttributeMacros.h"
#include "RiftPlayerAttributeSet.generated.h"

UCLASS()
class RIFT_API URiftPlayerAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	URiftPlayerAttributeSet();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_Health, Category="Attributes|Health")
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(URiftPlayerAttributeSet, Health)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_MaxHealth, Category="Attributes|Health")
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(URiftPlayerAttributeSet, MaxHealth)

	UPROPERTY(BlueprintReadOnly, Category="Attributes|Meta")
	FGameplayAttributeData Damage;
	ATTRIBUTE_ACCESSORS(URiftPlayerAttributeSet, Damage)

protected:
	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldHealth);

	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth);

};
