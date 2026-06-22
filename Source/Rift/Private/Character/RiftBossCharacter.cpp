#include "Character/RiftBossCharacter.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystem/RiftAbilitySystemComponent.h"
#include "AbilitySystem/Attributes/RiftEnemyAttributeSet.h"
#include "AbilitySystem/RiftGameplayTags.h"
#include "Components/SkeletalMeshComponent.h"
#include "Core/RiftGameplayGameMode.h"
#include "Net/UnrealNetwork.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"

ARiftBossCharacter::ARiftBossCharacter()
{
}

void ARiftBossCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent
			->GetGameplayAttributeValueChangeDelegate(URiftEnemyAttributeSet::GetHealthAttribute())
			.AddUObject(this, &ARiftBossCharacter::HandleHealthChanged);
		AbilitySystemComponent
			->GetGameplayAttributeValueChangeDelegate(URiftEnemyAttributeSet::GetMaxHealthAttribute())
			.AddUObject(this, &ARiftBossCharacter::HandleHealthChanged);
	}

	CheckCombatPhaseFromHealth();
}

void ARiftBossCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	StopBossPhase2Effect();

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent
			->GetGameplayAttributeValueChangeDelegate(URiftEnemyAttributeSet::GetHealthAttribute())
			.RemoveAll(this);
		AbilitySystemComponent
			->GetGameplayAttributeValueChangeDelegate(URiftEnemyAttributeSet::GetMaxHealthAttribute())
			.RemoveAll(this);
	}

	Super::EndPlay(EndPlayReason);
}

void ARiftBossCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ARiftBossCharacter, CurrentCombatPhase);
}

int32 ARiftBossCharacter::GetCurrentCombatPhase() const
{
	return CurrentCombatPhase;
}

bool ARiftBossCharacter::IsCombatPhaseTransitioningForAI() const
{
	return false;
}

void ARiftBossCharacter::HandleAttributeDeath(AActor* DeathInstigator)
{
	StopBossPhase2Effect();

	Super::HandleAttributeDeath(DeathInstigator);

	if (UWorld* World = GetWorld())
	{
		if (ARiftGameplayGameMode* GameplayGameMode = World->GetAuthGameMode<ARiftGameplayGameMode>())
		{
			GameplayGameMode->NotifyBossDefeated(this);
		}
	}
}

void ARiftBossCharacter::HandleAttributeDamageNumber(
	const float DamageAmount,
	const bool bBlocked,
	const FVector& WorldLocation
)
{
	Super::HandleAttributeDamageNumber(DamageAmount, bBlocked, WorldLocation);

	if (DamageAmount > 0.0f && !bBlocked)
	{
		CheckCombatPhaseFromHealth();
	}
}

void ARiftBossCharacter::OnRep_CombatPhase()
{
	OnBossCombatPhaseChanged(CurrentCombatPhase);

	if (CurrentCombatPhase >= 2)
	{
		StartBossPhase2Effect();
	}
}

void ARiftBossCharacter::HandleHealthChanged(const FOnAttributeChangeData& Data)
{
	CheckCombatPhaseFromHealth();
}

void ARiftBossCharacter::CheckCombatPhaseFromHealth()
{
	if (!HasAuthority() ||
		!bEnablePhase2 ||
		IsDeadForAI() ||
		CurrentCombatPhase >= 2 ||
		!AttributeSet)
	{
		return;
	}

	const float MaxHealth = AttributeSet->GetMaxHealth();
	if (MaxHealth <= 0.0f)
	{
		return;
	}

	const float HealthRatio = AttributeSet->GetHealth() / MaxHealth;
	if (HealthRatio <= Phase2HealthRatio)
	{
		EnterCombatPhase(2);
	}
}

void ARiftBossCharacter::EnterCombatPhase(const int32 NewPhase)
{
	if (!HasAuthority() || IsDeadForAI() || NewPhase <= CurrentCombatPhase)
	{
		return;
	}

	CurrentCombatPhase = NewPhase;
	OnBossCombatPhaseChanged(CurrentCombatPhase);

	ForceNetUpdate();

	if (NewPhase == 2)
	{
		Multicast_StartBossPhase2Effect();
	}
}

void ARiftBossCharacter::Multicast_StartBossPhase2Effect_Implementation()
{
	StartBossPhase2Effect();
}

void ARiftBossCharacter::StartBossPhase2Effect()
{
	StopBossPhase2Effect();
	if (!Phase2EffectNiagara)
	{
		return;
	}

	USkeletalMeshComponent* CharacterMesh = GetMesh();
	if (!CharacterMesh)
	{
		return;
	}

	ActivePhase2EffectComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
		Phase2EffectNiagara,
		CharacterMesh,
		Phase2EffectAttachSocketName,
		Phase2EffectLocationOffset,
		Phase2EffectRotationOffset,
		EAttachLocation::KeepRelativeOffset,
		true
	);

	if (ActivePhase2EffectComponent)
	{
		ActivePhase2EffectComponent->SetRelativeScale3D(Phase2EffectScale);
	}
}

void ARiftBossCharacter::StopBossPhase2Effect()
{
	if (ActivePhase2EffectComponent)
	{
		ActivePhase2EffectComponent->DestroyComponent();
		ActivePhase2EffectComponent = nullptr;
	}
}
