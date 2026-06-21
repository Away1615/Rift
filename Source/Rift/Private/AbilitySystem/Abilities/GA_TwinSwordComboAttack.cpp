#include "AbilitySystem/Abilities/GA_TwinSwordComboAttack.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/RiftGameplayTags.h"
#include "Animation/AnimInstance.h"
#include "Character/PlayerCharacter.h"
#include "Combat/RiftTargetAssistComponent.h"
#include "Combat/RiftWeaponTraceComponent.h"
#include "Data/Ability/TwinSwordComboAbilityConfig.h"
#include "Data/Player/Combat/RiftComboGraph.h"

UGA_TwinSwordComboAttack::UGA_TwinSwordComboAttack()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	bReplicateInputDirectly = true;

	FGameplayTagContainer AttackAssetTags;
	AttackAssetTags.AddTag(RiftGameplayTags::Ability_Attack_TwinSwordCombo);
	AttackAssetTags.AddTag(RiftGameplayTags::Ability_Attack_Combo);
	AttackAssetTags.AddTag(RiftGameplayTags::Ability_Attack_Primary);
	AttackAssetTags.AddTag(RiftGameplayTags::InputTag_Attack_Primary);
	SetAssetTags(AttackAssetTags);

	ActivationOwnedTags.AddTag(RiftGameplayTags::State_Attacking);
	ActivationBlockedTags.AddTag(RiftGameplayTags::State_Attacking);
	ActivationBlockedTags.AddTag(RiftGameplayTags::State_Hit_Light);
	ActivationBlockedTags.AddTag(RiftGameplayTags::State_Hit_Heavy);
	ActivationBlockedTags.AddTag(RiftGameplayTags::State_Dead);
}

bool UGA_TwinSwordComboAttack::CanActivateAbility(
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
	if (!AbilitySystemComponent)
	{
		return false;
	}

	return !AbilitySystemComponent->HasMatchingGameplayTag(RiftGameplayTags::State_Attack_RapidSlashReady);
}

void UGA_TwinSwordComboAttack::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData
)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	bIsFinishingAttack = false;
	ClearComboState();

	if (!ActorInfo)
	{
		UE_LOG(LogTemp, Warning, TEXT("TwinSwordCombo Activate failed: ActorInfo is invalid."));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(ActorInfo->AvatarActor.Get());
	if (!PlayerCharacter)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("TwinSwordCombo Activate failed: Avatar is not PlayerCharacter. Avatar=%s"),
			*GetNameSafe(ActorInfo->AvatarActor.Get())
		);
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	const FGameplayAbilitySpec* AbilitySpec = ActorInfo->AbilitySystemComponent.IsValid()
		? ActorInfo->AbilitySystemComponent->FindAbilitySpecFromHandle(Handle)
		: nullptr;
	ActiveComboConfig = AbilitySpec
		? Cast<UTwinSwordComboAbilityConfig>(AbilitySpec->SourceObject.Get())
		: nullptr;
	if (!ActiveComboConfig || !ActiveComboConfig->ComboGraph)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("TwinSwordCombo Activate failed: missing TwinSwordComboAbilityConfig or ComboGraph. Character=%s SourceObject=%s"),
			*GetNameSafe(PlayerCharacter),
			AbilitySpec ? *GetNameSafe(AbilitySpec->SourceObject.Get()) : TEXT("None")
		);
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	ActiveComboGraph = ActiveComboConfig->ComboGraph;
	ComboInputBufferDuration = FMath::Max(0.0f, ActiveComboConfig->ComboInputBufferDuration);

	ActiveAttackMontage = ActiveComboGraph->Montage;
	if (!ActiveAttackMontage)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("TwinSwordCombo Activate failed: ComboGraph has no Montage. ComboGraph=%s"),
			*GetNameSafe(ActiveComboGraph)
		);
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	const FGameplayTag EntryInputTag = RiftGameplayTags::InputTag_Attack_Primary;
	const FName EntrySection = ActiveComboGraph->GetEntrySection(EntryInputTag);

	if (EntrySection == NAME_None)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("TwinSwordCombo Activate failed: EntrySection is None. ComboGraph=%s InputTag=%s"),
			*GetNameSafe(ActiveComboGraph),
			*EntryInputTag.ToString()
		);
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	const FRiftComboNode* EntryNode = ActiveComboGraph->FindNode(EntrySection);
	if (!EntryNode)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("TwinSwordCombo Activate failed: Entry node missing. ComboGraph=%s EntrySection=%s InputTag=%s"),
			*GetNameSafe(ActiveComboGraph),
			*EntrySection.ToString(),
			*EntryInputTag.ToString()
		);
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (UAbilitySystemComponent* AbilitySystemComponent = ActorInfo->AbilitySystemComponent.Get())
	{
		FGameplayTagContainer GuardTags;
		GuardTags.AddTag(RiftGameplayTags::Ability_Guard);
		AbilitySystemComponent->CancelAbilities(&GuardTags, nullptr, this);
	}

	CurrentSection = EntrySection;
	PlayerCharacter->SetFacingMode(ERiftCharacterFacingMode::CombatAssist);
	EnterNode();

	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		NAME_None,
		ActiveAttackMontage,
		1.0f,
		CurrentSection,
		true
	);

	if (!MontageTask)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	MontageTask->OnCompleted.AddDynamic(this, &UGA_TwinSwordComboAttack::HandleMontageCompleted);
	MontageTask->OnBlendOut.AddDynamic(this, &UGA_TwinSwordComboAttack::HandleMontageBlendOut);
	MontageTask->OnInterrupted.AddDynamic(this, &UGA_TwinSwordComboAttack::HandleMontageInterrupted);
	MontageTask->OnCancelled.AddDynamic(this, &UGA_TwinSwordComboAttack::HandleMontageCancelled);
	MontageTask->ReadyForActivation();
}

void UGA_TwinSwordComboAttack::InputPressed(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo
)
{
	Super::InputPressed(Handle, ActorInfo, ActivationInfo);

	const FGameplayTag InputTag = RiftGameplayTags::InputTag_Attack_Primary;

	if (bComboChainWindowOpen)
	{
		PendingInputTag = InputTag;
		TryCommitComboChain();
	}
	else if (bComboInputWindowOpen)
	{
		PendingInputTag = InputTag;
	}
	else
	{
		PreBufferedInputTag = InputTag;
		const UWorld* World = GetWorld();
		const float CurrentTime = World ? World->GetTimeSeconds() : 0.0f;
		PreBufferedInputExpireTime = CurrentTime + ComboInputBufferDuration;
	}
}

void UGA_TwinSwordComboAttack::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const bool bReplicateEndAbility,
	const bool bWasCancelled
)
{
	if (ActorInfo)
	{
		if (APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(ActorInfo->AvatarActor.Get()))
		{
			PlayerCharacter->ClearActionCancelableState();
			PlayerCharacter->StopAssistedFacing();
			PlayerCharacter->SetFacingMode(ERiftCharacterFacingMode::Movement);
		}
	}

	ClearComboState();
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_TwinSwordComboAttack::OpenComboInputWindow()
{
	bComboInputWindowOpen = true;

	if (PreBufferedInputTag.IsValid())
	{
		const UWorld* World = GetWorld();
		const float CurrentTime = World ? World->GetTimeSeconds() : 0.0f;
		if (CurrentTime <= PreBufferedInputExpireTime)
		{
			PendingInputTag = PreBufferedInputTag;
		}
	}

	PreBufferedInputTag = FGameplayTag();
	PreBufferedInputExpireTime = 0.0f;
}

void UGA_TwinSwordComboAttack::CloseComboInputWindow()
{
	bComboInputWindowOpen = false;
}

void UGA_TwinSwordComboAttack::OpenComboChainWindow()
{
	bComboChainWindowOpen = true;
	bPendingRapidSlashChain = false;
	bRapidSlashChainCommitted = false;

	APlayerCharacter* PlayerCharacter = CurrentActorInfo
		? Cast<APlayerCharacter>(CurrentActorInfo->AvatarActor.Get())
		: nullptr;
	const FName ReadyGrantSection = GetRapidSlashReadyGrantSection();
	if (ReadyGrantSection != NAME_None && CurrentSection == ReadyGrantSection)
	{
		bPendingRapidSlashChain = true;
	}

	if (PreBufferedInputTag.IsValid())
	{
		const UWorld* World = GetWorld();
		const float CurrentTime = World ? World->GetTimeSeconds() : 0.0f;
		if (CurrentTime <= PreBufferedInputExpireTime)
		{
			PendingInputTag = PreBufferedInputTag;
		}

		PreBufferedInputTag = FGameplayTag();
		PreBufferedInputExpireTime = 0.0f;
	}

	if (PendingInputTag.IsValid())
	{
		TryCommitComboChain();
	}
}

void UGA_TwinSwordComboAttack::CloseComboChainWindow()
{
	bPendingRapidSlashChain = false;
	bRapidSlashChainCommitted = false;
	bComboChainWindowOpen = false;
}

bool UGA_TwinSwordComboAttack::TryCommitComboChain()
{
	if (!PendingInputTag.IsValid()) return false;

	const FGameplayTag InputTag = PendingInputTag;
	PendingInputTag = FGameplayTag();

	if (TryStartRapidSlashFromReadyGrantSection(InputTag))
	{
		return true;
	}

	if (!ActiveComboGraph) return false;

	const FRiftComboNode* CurrentNode = GetCurrentNode();
	if (!CurrentNode)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("TwinSwordCombo chain failed: CurrentNode missing. ComboGraph=%s CurrentSection=%s InputTag=%s"),
			*GetNameSafe(ActiveComboGraph),
			*CurrentSection.ToString(),
			*InputTag.ToString()
		);
		return false;
	}

	const FName NextSection = ResolveNextSection(*CurrentNode, InputTag);
	if (NextSection == NAME_None)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("TwinSwordCombo chain failed: no next section. ComboGraph=%s CurrentSection=%s InputTag=%s"),
			*GetNameSafe(ActiveComboGraph),
			*CurrentSection.ToString(),
			*InputTag.ToString()
		);
		return false;
	}

	const FRiftComboNode* NextNode = ActiveComboGraph->FindNode(NextSection);
	if (!NextNode)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("TwinSwordCombo chain failed: NextNode missing. ComboGraph=%s CurrentSection=%s InputTag=%s NextSection=%s"),
			*GetNameSafe(ActiveComboGraph),
			*CurrentSection.ToString(),
			*InputTag.ToString(),
			*NextSection.ToString()
		);
		return false;
	}

	PreBufferedInputTag = FGameplayTag();
	PreBufferedInputExpireTime = 0.0f;
	bComboInputWindowOpen = false;
	bComboChainWindowOpen = false;

	CurrentSection = NextSection;

	if (CurrentActorInfo && CurrentActorInfo->IsNetAuthority())
	{
		if (UAbilitySystemComponent* AbilitySystemComponent = CurrentActorInfo->AbilitySystemComponent.Get())
		{
			AbilitySystemComponent->CurrentMontageJumpToSection(NextSection);
		}
	}
	else if (ActiveAttackMontage && CurrentActorInfo)
	{
		if (UAnimInstance* AnimInstance = CurrentActorInfo->GetAnimInstance())
		{
			AnimInstance->Montage_JumpToSection(NextSection, ActiveAttackMontage);
		}
	}

	EnterNode();
	return true;
}

UGA_TwinSwordComboAttack* UGA_TwinSwordComboAttack::FindActiveTwinSwordComboInstance(AActor* AvatarActor)
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

		if (!AbilitySpec.Ability->GetAssetTags().HasTagExact(RiftGameplayTags::Ability_Attack_TwinSwordCombo))
		{
			continue;
		}

		return Cast<UGA_TwinSwordComboAttack>(AbilitySpec.GetPrimaryInstance());
	}

	return nullptr;
}

FName UGA_TwinSwordComboAttack::ResolveNextSection(
	const FRiftComboNode& CurrentNode,
	const FGameplayTag& InputTag
) const
{
	if (InputTag != RiftGameplayTags::InputTag_Attack_Primary)
	{
		return NAME_None;
	}

	for (const FRiftComboTransition& Transition : CurrentNode.Transitions)
	{
		if (Transition.InputTag == RiftGameplayTags::InputTag_Attack_Primary)
		{
			return Transition.ToSection;
		}
	}

	return NAME_None;
}

void UGA_TwinSwordComboAttack::HandleMontageCompleted()
{
	FinishAttackAbility(false);
}

void UGA_TwinSwordComboAttack::HandleMontageBlendOut()
{
	FinishAttackAbility(false);
}

void UGA_TwinSwordComboAttack::HandleMontageInterrupted()
{
	FinishAttackAbility(true);
}

void UGA_TwinSwordComboAttack::HandleMontageCancelled()
{
	FinishAttackAbility(true);
}

void UGA_TwinSwordComboAttack::FinishAttackAbility(const bool bWasCancelled)
{
	if (bIsFinishingAttack) return;

	if (!bWasCancelled)
	{
		GrantRapidSlashReadyFromConfig();
	}

	bIsFinishingAttack = true;
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, bWasCancelled);
}

void UGA_TwinSwordComboAttack::ClearComboState()
{
	CurrentSection = NAME_None;
	PendingInputTag = FGameplayTag();
	PreBufferedInputTag = FGameplayTag();
	PreBufferedInputExpireTime = 0.0f;
	bComboInputWindowOpen = false;
	bComboChainWindowOpen = false;
	bPendingRapidSlashChain = false;
	bRapidSlashChainCommitted = false;
	ComboInputBufferDuration = 0.25f;
	ActiveComboGraph = nullptr;
	ActiveAttackMontage = nullptr;
	ActiveComboConfig = nullptr;
}

void UGA_TwinSwordComboAttack::ApplyTargetAssistFacing()
{
	if (!CurrentActorInfo) return;

	APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(CurrentActorInfo->AvatarActor.Get());
	if (!PlayerCharacter) return;

	URiftTargetAssistComponent* TargetAssistComponent = PlayerCharacter->GetTargetAssistComponent();
	if (!TargetAssistComponent) return;

	FRotator DesiredFacing = FRotator::ZeroRotator;
	if (!TargetAssistComponent->GetDesiredFacing(DesiredFacing)) return;

	const float AssistFacingDuration = GetAssistFacingDuration();
	const float AssistFacingRotationSpeed = GetAssistFacingRotationSpeed();
	if (AssistFacingDuration <= 0.0f && AssistFacingRotationSpeed <= 0.0f) return;

	PlayerCharacter->StartAssistedFacing(
		DesiredFacing,
		AssistFacingDuration,
		AssistFacingRotationSpeed
	);
}

void UGA_TwinSwordComboAttack::UpdateWeaponTraceDamage()
{
	if (!CurrentActorInfo) return;

	APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(CurrentActorInfo->AvatarActor.Get());
	if (!PlayerCharacter) return;

	URiftWeaponTraceComponent* WeaponTraceComponent = PlayerCharacter->GetWeaponTraceComponent();
	if (!WeaponTraceComponent) return;

	const FRiftComboNode* Node = GetCurrentNode();
	if (!Node) return;

	WeaponTraceComponent->SetIncomingHitParams(
		Node->Damage,
		Node->PoiseDamage,
		Node->UltimateChargeOnHit
	);
	WeaponTraceComponent->SetIncomingCombatCueConfig(Node->CombatCueConfig);
	WeaponTraceComponent->SetIncomingHitStopConfig(Node->HitStopConfig);
	WeaponTraceComponent->SetIncomingCameraShake(Node->CameraShake, Node->CameraShakeDir);
}

void UGA_TwinSwordComboAttack::EnterNode()
{
	const FRiftComboNode* Node = GetCurrentNode();
	if (!Node) return;

	UpdateWeaponTraceDamage();
	ApplyTargetAssistFacing();
}

bool UGA_TwinSwordComboAttack::TryStartRapidSlashFromReadyGrantSection(const FGameplayTag& InputTag)
{
	if (!bComboChainWindowOpen || InputTag != RiftGameplayTags::InputTag_Attack_Primary)
	{
		return false;
	}

	if (!GrantRapidSlashReadyFromConfig())
	{
		return false;
	}

	UAbilitySystemComponent* AbilitySystemComponent = CurrentActorInfo
		? CurrentActorInfo->AbilitySystemComponent.Get()
		: nullptr;

	if (!AbilitySystemComponent)
	{
		return true;
	}

	FGameplayTagContainer RapidSlashAbilityTags;
	RapidSlashAbilityTags.AddTag(RiftGameplayTags::Ability_Attack_TwinSwordRapidSlash);
	bIsFinishingAttack = true;
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	const bool bActivatedRapidSlash = AbilitySystemComponent->TryActivateAbilitiesByTag(RapidSlashAbilityTags);
	bRapidSlashChainCommitted = bActivatedRapidSlash;
	return true;
}

bool UGA_TwinSwordComboAttack::GrantRapidSlashReadyFromConfig()
{
	APlayerCharacter* PlayerCharacter = CurrentActorInfo
		? Cast<APlayerCharacter>(CurrentActorInfo->AvatarActor.Get())
		: nullptr;
	const FName ReadyGrantSection = GetRapidSlashReadyGrantSection();
	if (ReadyGrantSection == NAME_None || CurrentSection != ReadyGrantSection)
	{
		return false;
	}

	SetRapidSlashReadyState(true);

	UWorld* World = GetWorld();
	if (!World) return true;

	World->GetTimerManager().ClearTimer(RapidSlashReadyTimerHandle);

	const float ReadyDuration = FMath::Max(0.0f, GetRapidSlashReadyDuration());
	if (ReadyDuration <= 0.0f)
	{
		ClearRapidSlashReadyState();
		return true;
	}

	World->GetTimerManager().SetTimer(
		RapidSlashReadyTimerHandle,
		this,
		&UGA_TwinSwordComboAttack::ClearRapidSlashReadyState,
		ReadyDuration,
		false
	);
	return true;
}

void UGA_TwinSwordComboAttack::SetRapidSlashReadyState(const bool bReady)
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

void UGA_TwinSwordComboAttack::ClearRapidSlashReadyState()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(RapidSlashReadyTimerHandle);
	}

	SetRapidSlashReadyState(false);
}

const FRiftComboNode* UGA_TwinSwordComboAttack::GetCurrentNode() const
{
	return ActiveComboGraph ? ActiveComboGraph->FindNode(CurrentSection) : nullptr;
}

float UGA_TwinSwordComboAttack::GetAssistFacingDuration() const
{
	return ActiveComboConfig ? ActiveComboConfig->AssistFacingDuration : 0.0f;
}

float UGA_TwinSwordComboAttack::GetAssistFacingRotationSpeed() const
{
	return ActiveComboConfig ? ActiveComboConfig->AssistFacingRotationSpeed : 0.0f;
}

FName UGA_TwinSwordComboAttack::GetRapidSlashReadyGrantSection() const
{
	return ActiveComboConfig ? ActiveComboConfig->RapidSlashReadyGrantSection : NAME_None;
}

float UGA_TwinSwordComboAttack::GetRapidSlashReadyDuration() const
{
	return ActiveComboConfig ? ActiveComboConfig->RapidSlashReadyDuration : 0.0f;
}
