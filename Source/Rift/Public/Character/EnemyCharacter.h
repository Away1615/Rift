// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "Character/BaseCharacter.h"
#include "EnemyCharacter.generated.h"

class UEnemyCharacterConfig;
class UAbilitySystemComponent;
class URiftAbilitySystemComponent;
class URiftEnemyAttributeSet;
/**
 *
 */
UCLASS()
class RIFT_API AEnemyCharacter : public ABaseCharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AEnemyCharacter();

	virtual void BeginPlay() override;
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Config")
	TObjectPtr<UEnemyCharacterConfig> EnemyCharacterConfig;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="AbilitySystem")
	TObjectPtr<URiftAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="AbilitySystem")
	TObjectPtr<URiftEnemyAttributeSet> AttributeSet;
};
