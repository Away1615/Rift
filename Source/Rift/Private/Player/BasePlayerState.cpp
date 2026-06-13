// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/BasePlayerState.h"

#include "AbilitySystem/RiftAbilitySystemComponent.h"
#include "AbilitySystem/RiftAttributeSet.h"

ABasePlayerState::ABasePlayerState()
{
	AbilitySystemComponent = CreateDefaultSubobject<URiftAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	AttributeSet = CreateDefaultSubobject<URiftAttributeSet>(TEXT("AttributeSet"));

	SetNetUpdateFrequency(100.0f);
}

UAbilitySystemComponent* ABasePlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}
