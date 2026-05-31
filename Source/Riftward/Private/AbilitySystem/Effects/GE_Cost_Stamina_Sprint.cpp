// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Effects/GE_Cost_Stamina_Sprint.h"

#include "AbilitySystem/BaseAttributeSet.h"

UGE_Cost_Stamina_Sprint::UGE_Cost_Stamina_Sprint()
{
	DurationPolicy = EGameplayEffectDurationType::Infinite;
	Period = 0.2f;
	bExecutePeriodicEffectOnApplication = false;

	FGameplayModifierInfo StaminaCost;
	StaminaCost.Attribute = UBaseAttributeSet::GetSPAttribute();
	StaminaCost.ModifierOp = EGameplayModOp::Additive;
	StaminaCost.ModifierMagnitude = FScalableFloat(-2.0f);

	Modifiers.Add(StaminaCost);
}
