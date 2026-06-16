#include "AbilitySystem/Abilities/GA_HeavyAttack.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/Attributes/RiftPlayerAttributeSet.h"
#include "AbilitySystem/Effects/GE_StaminaCost.h"
#include "AbilitySystem/RiftAbilitySystemComponent.h"
#include "AbilitySystem/RiftGameplayTags.h"
#include "Character/PlayerCharacter.h"
#include "Combat/RiftTargetAssistComponent.h"
#include "Combat/RiftWeaponTraceComponent.h"
#include "Data/Player/Combat/PlayerCombatConfig.h"
#include "Data/Player/PlayerClassConfig.h"

UGA_HeavyAttack::UGA_HeavyAttack()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	FGameplayTagContainer HeavyAttackAssetTags;
	HeavyAttackAssetTags.AddTag(RiftGameplayTags::InputTag_Attack_PrimaryHeavy);
	HeavyAttackAssetTags.AddTag(RiftGameplayTags::InputTag_Attack_SecondaryHeavy);
	HeavyAttackAssetTags.AddTag(RiftGameplayTags::Ability_Attack_Heavy);
	SetAssetTags(HeavyAttackAssetTags);

	ActivationOwnedTags.AddTag(RiftGameplayTags::State_Attacking);
	ActivationOwnedTags.AddTag(RiftGameplayTags::Ability_Attack_Heavy);
	ActivationBlockedTags.AddTag(RiftGameplayTags::Ability_Attack_Heavy);
	ActivationBlockedTags.AddTag(RiftGameplayTags::State_Dodging);
	ActivationBlockedTags.AddTag(RiftGameplayTags::State_Dead);
	ActivationBlockedTags.AddTag(RiftGameplayTags::State_Hit_Heavy);
	bReplicateInputDirectly = true;
}

void UGA_HeavyAttack::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData
)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	bIsFinishingHeavy = false;
	bHeavyStarted = false;

	if (!ActorInfo)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(ActorInfo->AvatarActor.Get());
	UAbilitySystemComponent* AbilitySystemComponent = ActorInfo->AbilitySystemComponent.Get();
	if (!PlayerCharacter || !AbilitySystemComponent)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	const UPlayerClassConfig* PlayerClassConfig = PlayerCharacter->GetPlayerClassConfig();
	const UPlayerCombatConfig* CombatConfig = PlayerClassConfig ? PlayerClassConfig->PlayerCombatConfig : nullptr;
	if (!CombatConfig)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	const URiftAbilitySystemComponent* RiftAbilitySystemComponent =
		Cast<URiftAbilitySystemComponent>(AbilitySystemComponent);
	const FGameplayTag PressedInputTag = RiftAbilitySystemComponent
		? RiftAbilitySystemComponent->LastPressedInputTag
		: FGameplayTag();
	const FRiftHeavyAttackConfig& Heavy = PressedInputTag == RiftGameplayTags::InputTag_Attack_SecondaryHeavy
		? CombatConfig->SecondaryHeavy
		: CombatConfig->PrimaryHeavy;

	if (!Heavy.Montage)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!TryChargeHeavyCost(Heavy.StaminaCost))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	FGameplayTagContainer AttackComboTags;
	AttackComboTags.AddTag(RiftGameplayTags::Ability_Attack_Combo);
	bHeavyStarted = true;
	AbilitySystemComponent->CancelAbilities(&AttackComboTags);

	PlayerCharacter->SetFacingMode(ERiftCharacterFacingMode::CombatAssist);
	if (URiftTargetAssistComponent* TargetAssistComponent = PlayerCharacter->GetTargetAssistComponent())
	{
		FRotator DesiredFacing = FRotator::ZeroRotator;
		if (TargetAssistComponent->GetDesiredFacing(DesiredFacing))
		{
			PlayerCharacter->StartAssistedFacing(
				DesiredFacing,
				CombatConfig->AssistFacingDuration,
				CombatConfig->AssistFacingRotationSpeed
			);
		}
	}

	if (URiftWeaponTraceComponent* WeaponTraceComponent = PlayerCharacter->GetWeaponTraceComponent())
	{
		WeaponTraceComponent->SetIncomingHitParams(
			Heavy.Damage,
			Heavy.PoiseDamage,
			Heavy.UltimateChargeOnHit
		);
		WeaponTraceComponent->SetIncomingCameraShake(Heavy.CameraShake, Heavy.CameraShakeDir);
	}

	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		NAME_None,
		Heavy.Montage,
		1.0f,
		NAME_None,
		true
	);
	if (!MontageTask)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	MontageTask->OnCompleted.AddDynamic(this, &UGA_HeavyAttack::HandleMontageCompleted);
	MontageTask->OnBlendOut.AddDynamic(this, &UGA_HeavyAttack::HandleMontageBlendOut);
	MontageTask->OnInterrupted.AddDynamic(this, &UGA_HeavyAttack::HandleMontageInterrupted);
	MontageTask->OnCancelled.AddDynamic(this, &UGA_HeavyAttack::HandleMontageCancelled);
	MontageTask->ReadyForActivation();
}

void UGA_HeavyAttack::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled
)
{
	if (bHeavyStarted && ActorInfo)
	{
		if (APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(ActorInfo->AvatarActor.Get()))
		{
			PlayerCharacter->StopAssistedFacing();
			PlayerCharacter->SetFacingMode(ERiftCharacterFacingMode::Movement);
		}
	}

	bHeavyStarted = false;
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_HeavyAttack::HandleMontageCompleted()
{
	FinishHeavyAbility(false);
}

void UGA_HeavyAttack::HandleMontageBlendOut()
{
	FinishHeavyAbility(false);
}

void UGA_HeavyAttack::HandleMontageInterrupted()
{
	FinishHeavyAbility(true);
}

void UGA_HeavyAttack::HandleMontageCancelled()
{
	FinishHeavyAbility(true);
}

bool UGA_HeavyAttack::TryChargeHeavyCost(const float StaminaCost)
{
	if (!CurrentActorInfo) return false;

	UAbilitySystemComponent* AbilitySystemComponent = CurrentActorInfo->AbilitySystemComponent.Get();
	if (!AbilitySystemComponent) return false;

	const float CurrentStamina = AbilitySystemComponent->GetNumericAttribute(
		URiftPlayerAttributeSet::GetStaminaAttribute()
	);
	if (CurrentStamina < StaminaCost) return false;

	if (StaminaCost > 0.0f)
	{
		FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
		FGameplayEffectSpecHandle StaminaCostSpec = AbilitySystemComponent->MakeOutgoingSpec(
			UGE_StaminaCost::StaticClass(),
			1.0f,
			EffectContext
		);
		if (StaminaCostSpec.IsValid())
		{
			StaminaCostSpec.Data->SetSetByCallerMagnitude(RiftGameplayTags::SetByCaller_StaminaCost, -StaminaCost);
			AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*StaminaCostSpec.Data.Get());
		}
	}

	return true;
}

void UGA_HeavyAttack::FinishHeavyAbility(const bool bWasCancelled)
{
	if (bIsFinishingHeavy) return;

	bIsFinishingHeavy = true;
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, bWasCancelled);
}
