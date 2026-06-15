// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Enemy/EnemyHealthBarWidget.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystem/Attributes/RiftEnemyAttributeSet.h"

void UEnemyHealthBarWidget::InitializeFor(UAbilitySystemComponent* InASC)
{
	if (bInitialized || !InASC) return;

	AbilitySystemComponent = InASC;
	BindToAbilitySystem(InASC);
	RefreshAll();
	bInitialized = true;
}

void UEnemyHealthBarWidget::NativeDestruct()
{
	if (UAbilitySystemComponent* ASC = AbilitySystemComponent.Get())
	{
		ASC->GetGameplayAttributeValueChangeDelegate(URiftEnemyAttributeSet::GetHealthAttribute()).RemoveAll(this);
		ASC->GetGameplayAttributeValueChangeDelegate(URiftEnemyAttributeSet::GetMaxHealthAttribute()).RemoveAll(this);
	}

	Super::NativeDestruct();
}

void UEnemyHealthBarWidget::BindToAbilitySystem(UAbilitySystemComponent* ASC)
{
	ASC->GetGameplayAttributeValueChangeDelegate(URiftEnemyAttributeSet::GetHealthAttribute()).AddUObject(
		this,
		&UEnemyHealthBarWidget::HandleHealthChanged
	);
	ASC->GetGameplayAttributeValueChangeDelegate(URiftEnemyAttributeSet::GetMaxHealthAttribute()).AddUObject(
		this,
		&UEnemyHealthBarWidget::HandleHealthChanged
	);
}

void UEnemyHealthBarWidget::RefreshAll()
{
	UAbilitySystemComponent* ASC = AbilitySystemComponent.Get();
	if (!ASC) return;

	OnHealthChanged(
		ASC->GetNumericAttribute(URiftEnemyAttributeSet::GetHealthAttribute()),
		ASC->GetNumericAttribute(URiftEnemyAttributeSet::GetMaxHealthAttribute())
	);
}

void UEnemyHealthBarWidget::HandleHealthChanged(const FOnAttributeChangeData&)
{
	UAbilitySystemComponent* ASC = AbilitySystemComponent.Get();
	if (!ASC) return;

	OnHealthChanged(
		ASC->GetNumericAttribute(URiftEnemyAttributeSet::GetHealthAttribute()),
		ASC->GetNumericAttribute(URiftEnemyAttributeSet::GetMaxHealthAttribute())
	);
}
