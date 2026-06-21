#include "AbilitySystem/Abilities/GA_TwinSwordRapidSlash.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/RiftGameplayTags.h"
#include "Animation/AnimInstance.h"
#include "Character/PlayerCharacter.h"
#include "Combat/RiftTargetAssistComponent.h"
#include "Combat/RiftWeaponTraceComponent.h"
#include "Data/Ability/TwinSwordRapidSlashAbilityConfig.h"
#include "Debug/Logger.h"

UGA_TwinSwordRapidSlash::UGA_TwinSwordRapidSlash()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	bReplicateInputDirectly = true;

	FGameplayTagContainer RapidSlashAssetTags;
	RapidSlashAssetTags.AddTag(RiftGameplayTags::Ability_Attack_TwinSwordRapidSlash);
	RapidSlashAssetTags.AddTag(RiftGameplayTags::InputTag_Attack_Primary);
	SetAssetTags(RapidSlashAssetTags);

	ActivationOwnedTags.AddTag(RiftGameplayTags::State_Attacking);

	ActivationBlockedTags.AddTag(RiftGameplayTags::State_Attacking);
	ActivationBlockedTags.AddTag(RiftGameplayTags::State_Hit_Light);
	ActivationBlockedTags.AddTag(RiftGameplayTags::State_Hit_Heavy);
	ActivationBlockedTags.AddTag(RiftGameplayTags::State_Dead);
}

bool UGA_TwinSwordRapidSlash::CanActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags,
	FGameplayTagContainer* OptionalRelevantTags
) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	const UAbilitySystemComponent* AbilitySystemComponent = ActorInfo
		? ActorInfo->AbilitySystemComponent.Get()
		: nullptr;
	if (!AbilitySystemComponent ||
		!AbilitySystemComponent->HasMatchingGameplayTag(RiftGameplayTags::State_Attack_RapidSlashReady))
	{
		return false;
	}

	return true;
}

void UGA_TwinSwordRapidSlash::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData
)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	ClearRapidSlashState();
	bIsFinishingRapidSlash = false;

	if (!ActorInfo)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	APlayerCharacter* PlayerCharacter = nullptr;
	FRiftResolvedTwinSwordRapidSlashConfig Config;
	if (!ResolveRapidSlashContext(PlayerCharacter, Config) || !PlayerCharacter)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!Config.Montage)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("TwinSwordRapidSlash Activate failed: Montage is not configured. Character=%s"),
			*GetNameSafe(PlayerCharacter)
		);
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (Config.SectionA == NAME_None)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("TwinSwordRapidSlash Activate failed: SectionA is None. Character=%s Montage=%s"),
			*GetNameSafe(PlayerCharacter),
			*GetNameSafe(Config.Montage)
		);
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (UAbilitySystemComponent* AbilitySystemComponent = ActorInfo->AbilitySystemComponent.Get())
	{
		FGameplayTagContainer GuardTags;
		GuardTags.AddTag(RiftGameplayTags::Ability_Guard);
		AbilitySystemComponent->CancelAbilities(&GuardTags, nullptr, this);
	}

	ActiveRapidSlashMontage = Config.Montage;
	CurrentRapidSection = Config.SectionA;
	SetRapidSlashReadyState(false);
	SetRapidSlashState(true);
	SetRapidSlashWeaponTrace(false);
	PlayerCharacter->ClearMovementInputCache();
	PlayerCharacter->StopAssistedFacing();
	PlayerCharacter->SetFacingMode(ERiftCharacterFacingMode::CombatAssist);
	ApplyTargetAssistFacing();

	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		NAME_None,
		ActiveRapidSlashMontage,
		FMath::Max(0.01f, Config.PlayRate),
		CurrentRapidSection,
		true
	);
	if (!MontageTask)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	MontageTask->OnCompleted.AddDynamic(this, &UGA_TwinSwordRapidSlash::HandleMontageCompleted);
	MontageTask->OnBlendOut.AddDynamic(this, &UGA_TwinSwordRapidSlash::HandleMontageBlendOut);
	MontageTask->OnInterrupted.AddDynamic(this, &UGA_TwinSwordRapidSlash::HandleMontageInterrupted);
	MontageTask->OnCancelled.AddDynamic(this, &UGA_TwinSwordRapidSlash::HandleMontageCancelled);
	MontageTask->ReadyForActivation();

	PlayerCharacter->SetActiveTwinSwordRapidSlashAbility(this);
	PlayerCharacter->Multicast_StartRapidSlashAuraVisual(
		Config.RapidSlashAuraNiagara,
		Config.RapidSlashAuraAttachSocketName,
		Config.RapidSlashAuraLocationOffset,
		Config.RapidSlashAuraRotationOffset,
		Config.RapidSlashAuraScale
	);
	FLogger::Log(PlayerCharacter, TEXT("RapidSlash Activated"), ELogSystem::Ability);
}

void UGA_TwinSwordRapidSlash::InputPressed(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo
)
{
	Super::InputPressed(Handle, ActorInfo, ActivationInfo);

	if (ActorInfo)
	{
		bPendingContinueInput = true;
		if (bComboChainWindowOpen)
		{
			TryCommitRapidSlashChain();
		}
	}
}

void UGA_TwinSwordRapidSlash::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const bool bReplicateEndAbility,
	const bool bWasCancelled
)
{
	SetRapidSlashState(false);
	SetRapidSlashReadyState(false);

	if (ActorInfo)
	{
		if (APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(ActorInfo->AvatarActor.Get()))
		{
			PlayerCharacter->ClearActionCancelableState();
			PlayerCharacter->ClearActiveTwinSwordRapidSlashAbility(this);
			PlayerCharacter->Multicast_StopRapidSlashAuraVisual();
			PlayerCharacter->StopAssistedFacing();
			PlayerCharacter->SetFacingMode(ERiftCharacterFacingMode::Movement);
		}
	}

	ClearRapidSlashState();
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_TwinSwordRapidSlash::OpenComboChainWindow()
{
	bComboChainWindowOpen = true;
	TryCommitRapidSlashChain();
}

void UGA_TwinSwordRapidSlash::CloseComboChainWindow()
{
	bComboChainWindowOpen = false;
}

void UGA_TwinSwordRapidSlash::RequestFinisherFromInput()
{
	if (bIsFinishingRapidSlash || bIsFinisherPlaying || CurrentRapidSection == NAME_None)
	{
		return;
	}

	APlayerCharacter* PlayerCharacter = nullptr;
	FRiftResolvedTwinSwordRapidSlashConfig Config;
	if (!ResolveRapidSlashContext(PlayerCharacter, Config) || !PlayerCharacter)
	{
		return;
	}

	if (Config.FinisherSection == NAME_None)
	{
		return;
	}

	SetRapidSlashReadyState(false);
	SetRapidSlashState(false);
	PlayerCharacter->Multicast_StopRapidSlashAuraVisual();

	bIsFinisherPlaying = true;
	bComboChainWindowOpen = false;
	bPendingContinueInput = false;
	bPendingFinisherInput = false;
	SetRapidSlashWeaponTrace(true);
	JumpToRapidSection(Config.FinisherSection);
}

UGA_TwinSwordRapidSlash* UGA_TwinSwordRapidSlash::FindActiveRapidSlashInstance(AActor* AvatarActor)
{
	if (!AvatarActor) return nullptr;

	APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(AvatarActor);
	if (!PlayerCharacter) return nullptr;

	UAbilitySystemComponent* AbilitySystemComponent = PlayerCharacter->GetAbilitySystemComponent();
	if (!AbilitySystemComponent) return nullptr;

	FScopedAbilityListLock AbilityListLock(*AbilitySystemComponent);
	for (FGameplayAbilitySpec& AbilitySpec : AbilitySystemComponent->GetActivatableAbilities())
	{
		if (!AbilitySpec.IsActive() || !AbilitySpec.Ability)
		{
			continue;
		}

		if (!AbilitySpec.Ability->GetAssetTags().HasTagExact(RiftGameplayTags::Ability_Attack_TwinSwordRapidSlash))
		{
			continue;
		}

		return Cast<UGA_TwinSwordRapidSlash>(AbilitySpec.GetPrimaryInstance());
	}

	return nullptr;
}

bool UGA_TwinSwordRapidSlash::ResolveRapidSlashContext(
	APlayerCharacter*& OutPlayerCharacter,
	FRiftResolvedTwinSwordRapidSlashConfig& OutConfig
) const
{
	OutPlayerCharacter = CurrentActorInfo
		? Cast<APlayerCharacter>(CurrentActorInfo->AvatarActor.Get())
		: nullptr;

	const FGameplayAbilitySpec* AbilitySpec =
		CurrentActorInfo && CurrentActorInfo->AbilitySystemComponent.IsValid()
			? CurrentActorInfo->AbilitySystemComponent->FindAbilitySpecFromHandle(CurrentSpecHandle)
			: nullptr;
	const UTwinSwordRapidSlashAbilityConfig* AbilityConfig = AbilitySpec
		? Cast<UTwinSwordRapidSlashAbilityConfig>(AbilitySpec->SourceObject.Get())
		: nullptr;
	if (AbilityConfig && AbilityConfig->Montage)
	{
		FillRapidSlashConfigFromAbilityConfig(AbilityConfig, OutConfig);
		return OutPlayerCharacter != nullptr;
	}

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("TwinSwordRapidSlash context failed: missing TwinSwordRapidSlashAbilityConfig or Montage. Character=%s SourceObject=%s"),
		*GetNameSafe(OutPlayerCharacter),
		AbilitySpec ? *GetNameSafe(AbilitySpec->SourceObject.Get()) : TEXT("None")
	);
	return false;
}

void UGA_TwinSwordRapidSlash::FillRapidSlashConfigFromAbilityConfig(
	const UTwinSwordRapidSlashAbilityConfig* AbilityConfig,
	FRiftResolvedTwinSwordRapidSlashConfig& OutConfig
) const
{
	if (!AbilityConfig) return;

	OutConfig.Montage = AbilityConfig->Montage;
	OutConfig.SectionA = AbilityConfig->SectionA;
	OutConfig.SectionB = AbilityConfig->SectionB;
	OutConfig.FinisherSection = AbilityConfig->FinisherSection;
	OutConfig.PlayRate = AbilityConfig->PlayRate;
	OutConfig.AssistFacingDuration = AbilityConfig->AssistFacingDuration;
	OutConfig.AssistFacingRotationSpeed = AbilityConfig->AssistFacingRotationSpeed;
	OutConfig.Damage = AbilityConfig->Damage;
	OutConfig.PoiseDamage = AbilityConfig->PoiseDamage;
	OutConfig.UltimateChargeOnHit = AbilityConfig->UltimateChargeOnHit;
	OutConfig.FinisherDamage = AbilityConfig->FinisherDamage;
	OutConfig.FinisherPoiseDamage = AbilityConfig->FinisherPoiseDamage;
	OutConfig.FinisherUltimateChargeOnHit = AbilityConfig->FinisherUltimateChargeOnHit;
	OutConfig.HitStopConfig = AbilityConfig->HitStopConfig;
	OutConfig.CombatCueConfig = AbilityConfig->CombatCueConfig;
	OutConfig.CameraShake = AbilityConfig->CameraShake;
	OutConfig.CameraShakeDir = AbilityConfig->CameraShakeDir;
	OutConfig.FinisherHitStopConfig = AbilityConfig->FinisherHitStopConfig;
	OutConfig.FinisherCombatCueConfig = AbilityConfig->FinisherCombatCueConfig;
	OutConfig.FinisherCameraShake = AbilityConfig->FinisherCameraShake;
	OutConfig.FinisherCameraShakeDir = AbilityConfig->FinisherCameraShakeDir;
	OutConfig.RapidSlashAuraNiagara = AbilityConfig->RapidSlashAuraNiagara;
	OutConfig.RapidSlashAuraAttachSocketName = AbilityConfig->RapidSlashAuraAttachSocketName;
	OutConfig.RapidSlashAuraLocationOffset = AbilityConfig->RapidSlashAuraLocationOffset;
	OutConfig.RapidSlashAuraRotationOffset = AbilityConfig->RapidSlashAuraRotationOffset;
	OutConfig.RapidSlashAuraScale = AbilityConfig->RapidSlashAuraScale;
}

void UGA_TwinSwordRapidSlash::ClearRapidSlashState()
{
	CurrentRapidSection = NAME_None;
	bPendingContinueInput = false;
	bPendingFinisherInput = false;
	bIsFinisherPlaying = false;
	bComboChainWindowOpen = false;
	ActiveRapidSlashMontage = nullptr;
}

void UGA_TwinSwordRapidSlash::FinishRapidSlashAbility(const bool bWasCancelled)
{
	if (bIsFinishingRapidSlash) return;

	bIsFinishingRapidSlash = true;
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, bWasCancelled);
}

void UGA_TwinSwordRapidSlash::SetRapidSlashState(const bool bReady)
{
	UAbilitySystemComponent* AbilitySystemComponent = CurrentActorInfo
		? CurrentActorInfo->AbilitySystemComponent.Get()
		: nullptr;
	if (!AbilitySystemComponent) return;

	const int32 NewCount = bReady ? 1 : 0;
	AbilitySystemComponent->SetLooseGameplayTagCount(RiftGameplayTags::State_Attack_RapidSlash, NewCount);
	if (AbilitySystemComponent->IsOwnerActorAuthoritative())
	{
		AbilitySystemComponent->SetReplicatedLooseGameplayTagCount(
			RiftGameplayTags::State_Attack_RapidSlash,
			NewCount
		);
	}
}

void UGA_TwinSwordRapidSlash::SetRapidSlashReadyState(const bool bReady)
{
	UAbilitySystemComponent* AbilitySystemComponent = CurrentActorInfo
		? CurrentActorInfo->AbilitySystemComponent.Get()
		: nullptr;
	if (!AbilitySystemComponent) return;

	const int32 NewCount = bReady ? 1 : 0;
	AbilitySystemComponent->SetLooseGameplayTagCount(RiftGameplayTags::State_Attack_RapidSlashReady, NewCount);
	if (AbilitySystemComponent->IsOwnerActorAuthoritative())
	{
		AbilitySystemComponent->SetReplicatedLooseGameplayTagCount(
			RiftGameplayTags::State_Attack_RapidSlashReady,
			NewCount
		);
	}
}

void UGA_TwinSwordRapidSlash::SetRapidSlashWeaponTrace(const bool bUseFinisherParams)
{
	APlayerCharacter* PlayerCharacter = nullptr;
	FRiftResolvedTwinSwordRapidSlashConfig Config;
	if (!ResolveRapidSlashContext(PlayerCharacter, Config) || !PlayerCharacter) return;

	URiftWeaponTraceComponent* WeaponTraceComponent = PlayerCharacter->GetWeaponTraceComponent();
	if (!WeaponTraceComponent) return;

	if (bUseFinisherParams)
	{
		WeaponTraceComponent->SetIncomingHitParams(
			Config.FinisherDamage,
			Config.FinisherPoiseDamage,
			Config.FinisherUltimateChargeOnHit
		);
		WeaponTraceComponent->SetIncomingCombatCueConfig(Config.FinisherCombatCueConfig);
		WeaponTraceComponent->SetIncomingHitStopConfig(Config.FinisherHitStopConfig);
		WeaponTraceComponent->SetIncomingCameraShake(
			Config.FinisherCameraShake,
			Config.FinisherCameraShakeDir
		);
		return;
	}

	WeaponTraceComponent->SetIncomingHitParams(
		Config.Damage,
		Config.PoiseDamage,
		Config.UltimateChargeOnHit
	);
	WeaponTraceComponent->SetIncomingCombatCueConfig(Config.CombatCueConfig);
	WeaponTraceComponent->SetIncomingHitStopConfig(Config.HitStopConfig);
	WeaponTraceComponent->SetIncomingCameraShake(Config.CameraShake, Config.CameraShakeDir);
}

void UGA_TwinSwordRapidSlash::ApplyTargetAssistFacing()
{
	if (!CurrentActorInfo) return;

	APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(CurrentActorInfo->AvatarActor.Get());
	if (!PlayerCharacter) return;

	URiftTargetAssistComponent* TargetAssistComponent = PlayerCharacter->GetTargetAssistComponent();
	if (!TargetAssistComponent) return;

	FRotator DesiredFacing = FRotator::ZeroRotator;
	if (!TargetAssistComponent->GetDesiredFacing(DesiredFacing)) return;

	FRiftResolvedTwinSwordRapidSlashConfig Config;
	if (!ResolveRapidSlashContext(PlayerCharacter, Config)) return;

	PlayerCharacter->StartAssistedFacing(
		DesiredFacing,
		Config.AssistFacingDuration,
		Config.AssistFacingRotationSpeed
	);
}

bool UGA_TwinSwordRapidSlash::TryCommitRapidSlashChain()
{
	if (bIsFinisherPlaying) return false;

	APlayerCharacter* PlayerCharacter = nullptr;
	FRiftResolvedTwinSwordRapidSlashConfig Config;
	if (!ResolveRapidSlashContext(PlayerCharacter, Config)) return false;

	if (bPendingFinisherInput)
	{
		if (Config.FinisherSection == NAME_None)
		{
			FinishRapidSlashAbility(false);
			return false;
		}

		bIsFinisherPlaying = true;
		bComboChainWindowOpen = false;
		bPendingContinueInput = false;
		bPendingFinisherInput = false;
		SetRapidSlashWeaponTrace(true);
		JumpToRapidSection(Config.FinisherSection);
		return true;
	}

	if (bPendingContinueInput)
	{
		const FName NextSection = ResolveNextLoopSection(Config);
		if (NextSection == NAME_None)
		{
			FinishRapidSlashAbility(false);
			return false;
		}

		bComboChainWindowOpen = false;
		bPendingContinueInput = false;
		SetRapidSlashWeaponTrace(false);
		JumpToRapidSection(NextSection);
		return true;
	}

	return false;
}

void UGA_TwinSwordRapidSlash::JumpToRapidSection(const FName SectionName)
{
	if (SectionName == NAME_None) return;

	CurrentRapidSection = SectionName;
	if (CurrentActorInfo && CurrentActorInfo->IsNetAuthority())
	{
		if (UAbilitySystemComponent* AbilitySystemComponent = CurrentActorInfo->AbilitySystemComponent.Get())
		{
			AbilitySystemComponent->CurrentMontageJumpToSection(SectionName);
		}
	}
	else if (ActiveRapidSlashMontage && CurrentActorInfo)
	{
		if (UAnimInstance* AnimInstance = CurrentActorInfo->GetAnimInstance())
		{
			AnimInstance->Montage_JumpToSection(SectionName, ActiveRapidSlashMontage);
		}
	}

	ApplyTargetAssistFacing();
}

FName UGA_TwinSwordRapidSlash::ResolveNextLoopSection(
	const FRiftResolvedTwinSwordRapidSlashConfig& Config
) const
{
	if (CurrentRapidSection == Config.SectionA)
	{
		return Config.SectionB;
	}

	if (CurrentRapidSection == Config.SectionB)
	{
		return Config.SectionA;
	}

	return Config.SectionA;
}

void UGA_TwinSwordRapidSlash::HandleMontageCompleted()
{
	FinishRapidSlashAbility(false);
}

void UGA_TwinSwordRapidSlash::HandleMontageBlendOut()
{
	FinishRapidSlashAbility(false);
}

void UGA_TwinSwordRapidSlash::HandleMontageInterrupted()
{
	FinishRapidSlashAbility(true);
}

void UGA_TwinSwordRapidSlash::HandleMontageCancelled()
{
	FinishRapidSlashAbility(true);
}
