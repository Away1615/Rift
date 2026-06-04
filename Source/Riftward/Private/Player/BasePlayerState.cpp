// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/BasePlayerState.h"
#include "AbilitySystem/Attributes/HealthAttributeSet.h"
#include "AbilitySystem/Attributes/ResourceAttributeSet.h"

ABasePlayerState::ABasePlayerState()
{
	AbilitySystemComponent = CreateDefaultSubobject<UBaseAbilitySystemComponent>("AbilitySystemComponent");
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	HealthAttributeSet = CreateDefaultSubobject<UHealthAttributeSet>("HealthAttributeSet");
	ResourceAttributeSet = CreateDefaultSubobject<UResourceAttributeSet>("ResourceAttributeSet");

	SetNetUpdateFrequency(100.0f);
}

UAbilitySystemComponent* ABasePlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}
