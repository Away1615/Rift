// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/EnemyCharacter.h"

#include "AbilitySystem/RiftAbilitySystemComponent.h"
#include "AbilitySystem/Attributes/RiftEnemyAttributeSet.h"

AEnemyCharacter::AEnemyCharacter()
{
	bReplicates = true;
	ACharacter::SetReplicateMovement(true);

	AbilitySystemComponent = CreateDefaultSubobject<URiftAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);

	AttributeSet = CreateDefaultSubobject<URiftEnemyAttributeSet>(TEXT("AttributeSet"));
}

void AEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();

	// The ASC is created in the constructor, so it must always exist on a properly
	// instantiated actor. A null here means a stale level instance placed before the
	// component was added to the class. Fix the asset by re-placing it.
	if (!ensureMsgf(AbilitySystemComponent, TEXT("%s has no AbilitySystemComponent. Re-place this actor in the level."), *GetName()))
	{
		return;
	}

	AbilitySystemComponent->InitAbilityActorInfo(this, this);
}

UAbilitySystemComponent* AEnemyCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}
