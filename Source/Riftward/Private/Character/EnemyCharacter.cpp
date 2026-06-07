// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/EnemyCharacter.h"

#include "AbilitySystem/Attributes/EnemyAttributeSet.h"
#include "AbilitySystem/Attributes/HealthAttributeSet.h"
#include "Data/Enemy/Common/EnemyCommonConfig.h"
#include "Data/Enemy/EnemyCharacterConfig.h"

AEnemyCharacter::AEnemyCharacter()
{
	bReplicates = true;
	ACharacter::SetReplicateMovement(true);

	AbilitySystemComponent = CreateDefaultSubobject<UBaseAbilitySystemComponent>("AbilitySystemComponent");
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);

	HealthAttributeSet = CreateDefaultSubobject<UHealthAttributeSet>("HealthAttributeSet");
	EnemyAttributeSet = CreateDefaultSubobject<UEnemyAttributeSet>("EnemyAttributeSet");
}

UAbilitySystemComponent* AEnemyCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();

	AbilitySystemComponent->InitAbilityActorInfo(this, this);
	ApplyEnemyConfig();
}

void AEnemyCharacter::ApplyEnemyConfig() const
{
	if (!HasAuthority() || !EnemyCharacterConfig || !EnemyCharacterConfig->EnemyCommonConfig) return;

	const UEnemyCommonConfig* CommonConfig = EnemyCharacterConfig->EnemyCommonConfig;
	if (HealthAttributeSet)
	{
		HealthAttributeSet->SetMaxHealth(CommonConfig->MaxHealth);
		HealthAttributeSet->SetHealth(FMath::Clamp(CommonConfig->Health, 0.0f, CommonConfig->MaxHealth));
	}

	if (EnemyAttributeSet)
	{
		EnemyAttributeSet->SetMaxPoise(CommonConfig->MaxPoise);
		EnemyAttributeSet->SetPoise(FMath::Clamp(CommonConfig->Poise, 0.0f, CommonConfig->MaxPoise));
		EnemyAttributeSet->SetMaxRage(CommonConfig->MaxRage);
		EnemyAttributeSet->SetRage(FMath::Clamp(CommonConfig->Rage, 0.0f, CommonConfig->MaxRage));
	}
}
