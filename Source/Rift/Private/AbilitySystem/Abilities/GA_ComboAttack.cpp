#include "AbilitySystem/Abilities/GA_ComboAttack.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/Attributes/RiftPlayerAttributeSet.h"
#include "AbilitySystem/Effects/GE_StaminaCost.h"
#include "AbilitySystem/RiftAbilitySystemComponent.h"
#include "AbilitySystem/RiftGameplayTags.h"
#include "Animation/AnimInstance.h"
#include "Character/PlayerCharacter.h"
#include "Combat/RiftTargetAssistComponent.h"
#include "Combat/RiftWeaponTraceComponent.h"
#include "Data/Player/Combat/PlayerCombatConfig.h"
#include "Data/Player/Combat/RiftComboGraph.h"
#include "Data/Player/PlayerClassConfig.h"

UGA_ComboAttack::UGA_ComboAttack()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	FGameplayTagContainer ComboAttackAssetTags;
	ComboAttackAssetTags.AddTag(RiftGameplayTags::Ability_Attack_Primary);
	ComboAttackAssetTags.AddTag(RiftGameplayTags::Ability_Attack_Secondary);
	ComboAttackAssetTags.AddTag(RiftGameplayTags::Ability_Attack_Combo);
	ComboAttackAssetTags.AddTag(RiftGameplayTags::InputTag_Attack_Primary);
	ComboAttackAssetTags.AddTag(RiftGameplayTags::InputTag_Attack_Secondary);
	SetAssetTags(ComboAttackAssetTags);

	ActivationOwnedTags.AddTag(RiftGameplayTags::State_Attacking);
	ActivationBlockedTags.AddTag(RiftGameplayTags::State_Attacking);
	ActivationBlockedTags.AddTag(RiftGameplayTags::State_Dead);
	ActivationBlockedTags.AddTag(RiftGameplayTags::State_Hit_Heavy);
	bReplicateInputDirectly = true;
}

void UGA_ComboAttack::ActivateAbility(
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
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	const UPlayerClassConfig* PlayerClassConfig = PlayerCharacter->GetPlayerClassConfig();
	const UPlayerCombatConfig* PlayerCombatConfig = PlayerClassConfig ? PlayerClassConfig->PlayerCombatConfig : nullptr;
	if (!PlayerCombatConfig || !PlayerCombatConfig->ComboGraph)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	ActiveComboGraph = PlayerCombatConfig->ComboGraph;
	ActiveAttackMontage = ActiveComboGraph->Montage;
	ComboInputBufferDuration = FMath::Max(0.0f, PlayerCombatConfig->ComboInputBufferDuration);
	if (!ActiveAttackMontage)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	const URiftAbilitySystemComponent* RiftAbilitySystemComponent =
		Cast<URiftAbilitySystemComponent>(ActorInfo->AbilitySystemComponent.Get());
	FGameplayTag EntryInputTag = RiftAbilitySystemComponent
		? RiftAbilitySystemComponent->LastPressedInputTag
		: FGameplayTag();
	if (!EntryInputTag.IsValid())
	{
		EntryInputTag = RiftGameplayTags::InputTag_Attack_Primary;
	}

	const FName EntrySection = ActiveComboGraph->GetEntrySection(EntryInputTag);
	if (EntrySection == NAME_None)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	const FRiftComboNode* EntryNode = ActiveComboGraph->FindNode(EntrySection);
	if (!EntryNode)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (EntryNode->StaminaCost > 0.0f && !TryChargeStamina(EntryNode->StaminaCost))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
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

	MontageTask->OnCompleted.AddDynamic(this, &UGA_ComboAttack::HandleMontageCompleted);
	MontageTask->OnBlendOut.AddDynamic(this, &UGA_ComboAttack::HandleMontageBlendOut);
	MontageTask->OnInterrupted.AddDynamic(this, &UGA_ComboAttack::HandleMontageInterrupted);
	MontageTask->OnCancelled.AddDynamic(this, &UGA_ComboAttack::HandleMontageCancelled);
	MontageTask->ReadyForActivation();
}

void UGA_ComboAttack::InputPressed(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo
)
{
	Super::InputPressed(Handle, ActorInfo, ActivationInfo);

	const URiftAbilitySystemComponent* RiftAbilitySystemComponent = ActorInfo
		? Cast<URiftAbilitySystemComponent>(ActorInfo->AbilitySystemComponent.Get())
		: nullptr;
	FGameplayTag InputTag = RiftAbilitySystemComponent
		? RiftAbilitySystemComponent->LastPressedInputTag
		: FGameplayTag();
	if (!InputTag.IsValid())
	{
		InputTag = RiftGameplayTags::InputTag_Attack_Primary;
	}

	if (bComboInputWindowOpen)
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

void UGA_ComboAttack::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled
)
{
	if (ActorInfo)
	{
		if (APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(ActorInfo->AvatarActor.Get()))
		{
			PlayerCharacter->StopAssistedFacing();
			PlayerCharacter->SetFacingMode(ERiftCharacterFacingMode::Movement);
		}
	}

	ClearComboState();
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_ComboAttack::OpenComboInputWindow()
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

void UGA_ComboAttack::CloseComboInputWindow()
{
	bComboInputWindowOpen = false;
}

void UGA_ComboAttack::CommitComboChainPoint()
{
	if (!PendingInputTag.IsValid()) return;

	if (!ActiveComboGraph)
	{
		PendingInputTag = FGameplayTag();
		return;
	}

	const FRiftComboNode* CurrentNode = GetCurrentNode();
	if (!CurrentNode)
	{
		PendingInputTag = FGameplayTag();
		return;
	}

	FName NextSection = NAME_None;
	for (const FRiftComboTransition& Transition : CurrentNode->Transitions)
	{
		if (Transition.InputTag == PendingInputTag)
		{
			NextSection = Transition.ToSection;
			break;
		}
	}

	PendingInputTag = FGameplayTag();
	bComboInputWindowOpen = false;
	PreBufferedInputTag = FGameplayTag();
	PreBufferedInputExpireTime = 0.0f;

	if (NextSection == NAME_None) return;

	const FRiftComboNode* NextNode = ActiveComboGraph->FindNode(NextSection);
	if (!NextNode) return;

	if (NextNode->StaminaCost > 0.0f && !TryChargeStamina(NextNode->StaminaCost)) return;

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
}

UGA_ComboAttack* UGA_ComboAttack::FindActiveComboInstance(AActor* AvatarActor)
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

		if (!AbilitySpec.Ability->GetAssetTags().HasTagExact(RiftGameplayTags::Ability_Attack_Combo))
		{
			continue;
		}

		return Cast<UGA_ComboAttack>(AbilitySpec.GetPrimaryInstance());
	}

	return nullptr;
}

void UGA_ComboAttack::HandleMontageCompleted()
{
	FinishAttackAbility(false);
}

void UGA_ComboAttack::HandleMontageBlendOut()
{
	FinishAttackAbility(false);
}

void UGA_ComboAttack::HandleMontageInterrupted()
{
	FinishAttackAbility(true);
}

void UGA_ComboAttack::HandleMontageCancelled()
{
	FinishAttackAbility(true);
}

void UGA_ComboAttack::FinishAttackAbility(const bool bWasCancelled)
{
	if (bIsFinishingAttack) return;

	bIsFinishingAttack = true;
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, bWasCancelled);
}

void UGA_ComboAttack::ClearComboState()
{
	CurrentSection = NAME_None;
	PendingInputTag = FGameplayTag();
	PreBufferedInputTag = FGameplayTag();
	PreBufferedInputExpireTime = 0.0f;
	bComboInputWindowOpen = false;
	ComboInputBufferDuration = 0.25f;
	ActiveComboGraph = nullptr;
	ActiveAttackMontage = nullptr;
}

void UGA_ComboAttack::ApplyTargetAssistFacing()
{
	if (!CurrentActorInfo) return;

	APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(CurrentActorInfo->AvatarActor.Get());
	if (!PlayerCharacter) return;

	URiftTargetAssistComponent* TargetAssistComponent = PlayerCharacter->GetTargetAssistComponent();
	if (!TargetAssistComponent) return;

	FRotator DesiredFacing = FRotator::ZeroRotator;
	if (!TargetAssistComponent->GetDesiredFacing(DesiredFacing)) return;

	const UPlayerClassConfig* PlayerClassConfig = PlayerCharacter->GetPlayerClassConfig();
	const UPlayerCombatConfig* PlayerCombatConfig = PlayerClassConfig ? PlayerClassConfig->PlayerCombatConfig : nullptr;
	if (!PlayerCombatConfig) return;

	PlayerCharacter->StartAssistedFacing(
		DesiredFacing,
		PlayerCombatConfig->AssistFacingDuration,
		PlayerCombatConfig->AssistFacingRotationSpeed
	);
}

void UGA_ComboAttack::UpdateWeaponTraceDamage()
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
	WeaponTraceComponent->SetIncomingCameraShake(Node->CameraShake, Node->CameraShakeDir);
}

void UGA_ComboAttack::EnterNode()
{
	ApplyTargetAssistFacing();
	UpdateWeaponTraceDamage();
}

bool UGA_ComboAttack::TryChargeStamina(const float Cost)
{
	if (Cost <= 0.0f) return true;
	if (!CurrentActorInfo) return false;

	UAbilitySystemComponent* AbilitySystemComponent = CurrentActorInfo->AbilitySystemComponent.Get();
	if (!AbilitySystemComponent) return false;

	const float CurrentStamina = AbilitySystemComponent->GetNumericAttribute(
		URiftPlayerAttributeSet::GetStaminaAttribute()
	);
	if (CurrentStamina < Cost) return false;

	FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
	FGameplayEffectSpecHandle StaminaCostSpec = AbilitySystemComponent->MakeOutgoingSpec(
		UGE_StaminaCost::StaticClass(),
		1.0f,
		EffectContext
	);
	if (StaminaCostSpec.IsValid())
	{
		StaminaCostSpec.Data->SetSetByCallerMagnitude(RiftGameplayTags::SetByCaller_StaminaCost, -Cost);
		AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*StaminaCostSpec.Data.Get());
	}

	return true;
}

const FRiftComboNode* UGA_ComboAttack::GetCurrentNode() const
{
	return ActiveComboGraph ? ActiveComboGraph->FindNode(CurrentSection) : nullptr;
}
