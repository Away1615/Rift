// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/GameplayAbilities/GA_TwinSword_Combo.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Abilities/Tasks/AbilityTask_WaitInputPress.h"
#include "Animation/AnimInstance.h"
#include "Character/PlayerCharacter.h"
#include "Character/EnemyCharacter.h"
#include "AbilitySystem/GameplayEffects/GE_InstantDamage.h"
#include "Data/Player/Ability/PlayerAbilitySetConfig.h"
#include "Debug/Logger.h"
#include "DrawDebugHelpers.h"
#include "AbilitySystem/Attributes/HealthAttributeSet.h"
#include "Equipment/PlayerWeapon.h"
#include "GameplayTags/RiftGameplayTags.h"

void UGA_TwinSword_Combo::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                          const FGameplayAbilityActorInfo* ActorInfo,
                                          const FGameplayAbilityActivationInfo ActivationInfo,
                                          const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(GetAvatarActorFromActorInfo());
	if (!PlayerCharacter)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	const FPlayerAbilityEntry* ComboEntry = GetAbilityEntry();
	const URiftAbilityMontageFragment* MontageFragment = ComboEntry
		? ComboEntry->FindFragment<URiftAbilityMontageFragment>()
		: nullptr;
	const URiftAbilityComboFragment* ComboFragment = ComboEntry
		? ComboEntry->FindFragment<URiftAbilityComboFragment>()
		: nullptr;
	if (!ComboEntry || !MontageFragment || !ComboFragment)
	{
		Logger::Error(PlayerCharacter, TEXT("TwinSword combo fragments are missing"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo();
	if (!AbilitySystemComponent)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	const FRiftGameplayTags& RiftTags = FRiftGameplayTags::Get();
	const bool bUseSuperAttack = AbilitySystemComponent
		&& AbilitySystemComponent->HasMatchingGameplayTag(RiftTags.State_Ability_TwinSword_Core_PerfectDodgeEmpowered);

	UAnimMontage* MontageToPlay = bUseSuperAttack && MontageFragment->EmpoweredMontage
		? MontageFragment->EmpoweredMontage
		: MontageFragment->Montage;

	if (!MontageToPlay)
	{
		Logger::Error(PlayerCharacter, bUseSuperAttack
			? TEXT("TwinSword super attack montage is missing")
			: TEXT("TwinSword attack montage is missing"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (ComboFragment->Steps.IsEmpty())
	{
		Logger::Error(PlayerCharacter, TEXT("TwinSword combo sections are missing"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (ShouldCommitComboAbility() && !CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (bUseSuperAttack)
	{
		AbilitySystemComponent->RemoveLooseGameplayTag(RiftTags.State_Ability_TwinSword_Core_PerfectDodgeEmpowered);
	}
	bCurrentAttackEmpowered = bUseSuperAttack;

	AbilitySystemComponent->AddLooseGameplayTag(GetAbilityActiveStateTag());
	bAddedComboActiveTag = true;

	ActiveMontage = MontageToPlay;
	ActiveComboSections.Reset();
	for (const FRiftComboStepSpec& ComboStep : ComboFragment->Steps)
	{
		if (!ComboStep.SectionName.IsNone())
		{
			ActiveComboSections.Add(ComboStep.SectionName);
		}
	}

	if (ActiveComboSections.IsEmpty())
	{
		Logger::Error(PlayerCharacter, TEXT("TwinSword combo section names are missing"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	bHasBufferedInput = false;
	bCanConsumeBufferedInput = false;

	const FName StartSection = MontageFragment->StartSection.IsNone()
		? ActiveComboSections[0]
		: MontageFragment->StartSection;

	UAbilityTask_PlayMontageAndWait* MontageTask =
		UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this,
			NAME_None,
			ActiveMontage,
			MontageFragment->PlayRate,
			StartSection,
			true
		);

	MontageTask->OnCompleted.AddDynamic(this, &UGA_TwinSword_Combo::HandleMontageCompleted);
	MontageTask->OnCancelled.AddDynamic(this, &UGA_TwinSword_Combo::HandleMontageCancelled);
	MontageTask->OnInterrupted.AddDynamic(this, &UGA_TwinSword_Combo::HandleMontageInterrupted);

	ResetComboSectionLinks();
	WaitForComboWindow();
	WaitForComboInput();
	WaitForWeaponTraceEvents();
	MontageTask->ReadyForActivation();
}

void UGA_TwinSword_Combo::EndAbility(const FGameplayAbilitySpecHandle Handle,
                                     const FGameplayAbilityActorInfo* ActorInfo,
                                     const FGameplayAbilityActivationInfo ActivationInfo,
                                     bool bReplicateEndAbility,
                                     bool bWasCancelled)
{
	if (bAddedComboActiveTag)
	{
		if (UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo())
		{
			AbilitySystemComponent->RemoveLooseGameplayTag(GetAbilityActiveStateTag());
		}
	}

	ActiveMontage = nullptr;
	ActiveComboSections.Empty();
	bHasBufferedInput = false;
	bCanConsumeBufferedInput = false;
	bAddedComboActiveTag = false;
	bCurrentAttackEmpowered = false;
	ResetWeaponTrace();

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

FGameplayTag UGA_TwinSword_Combo::GetComboWindowEventTag() const
{
	return FGameplayTag();
}

FGameplayTag UGA_TwinSword_Combo::GetAbilityActiveStateTag() const
{
	return FGameplayTag();
}

bool UGA_TwinSword_Combo::ShouldCommitComboAbility() const
{
	return false;
}

bool UGA_TwinSword_Combo::CheckCost(const FGameplayAbilitySpecHandle Handle,
                                    const FGameplayAbilityActorInfo* ActorInfo,
                                    FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!ShouldCommitComboAbility())
	{
		return Super::CheckCost(Handle, ActorInfo, OptionalRelevantTags);
	}

	return Super::CheckCost(Handle, ActorInfo, OptionalRelevantTags);
}

void UGA_TwinSword_Combo::ApplyCost(const FGameplayAbilitySpecHandle Handle,
                                    const FGameplayAbilityActorInfo* ActorInfo,
                                    const FGameplayAbilityActivationInfo ActivationInfo) const
{
	if (!ShouldCommitComboAbility())
	{
		Super::ApplyCost(Handle, ActorInfo, ActivationInfo);
		return;
	}

	Super::ApplyCost(Handle, ActorInfo, ActivationInfo);
}

void UGA_TwinSword_Combo::ResetComboSectionLinks() const
{
	UAnimInstance* AnimInstance = CurrentActorInfo
		? CurrentActorInfo->GetAnimInstance()
		: nullptr;

	if (!AnimInstance || !ActiveMontage) return;

	UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo();

	for (const FName ComboSection : ActiveComboSections)
	{
		if (CurrentActorInfo->IsNetAuthority() && AbilitySystemComponent)
		{
			AbilitySystemComponent->CurrentMontageSetNextSectionName(ComboSection, NAME_None);
		}
		else
		{
			AnimInstance->Montage_SetNextSection(ComboSection, NAME_None, ActiveMontage);
		}
	}
}

void UGA_TwinSword_Combo::WaitForComboWindow()
{
	const FGameplayTag ComboWindowEventTag = GetComboWindowEventTag();
	if (!ComboWindowEventTag.IsValid()) return;

	UAbilityTask_WaitGameplayEvent* ComboWindowTask =
		UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
			this,
			ComboWindowEventTag,
			nullptr,
			false,
			true
		);

	ComboWindowTask->EventReceived.AddDynamic(
		this,
		&UGA_TwinSword_Combo::HandleComboWindow
	);

	ComboWindowTask->ReadyForActivation();
}

void UGA_TwinSword_Combo::WaitForComboInput()
{
	UAbilityTask_WaitInputPress* ComboInputTask =
		UAbilityTask_WaitInputPress::WaitInputPress(this, false);

	ComboInputTask->OnPress.AddDynamic(
		this,
		&UGA_TwinSword_Combo::HandleComboInputPressed
	);

	ComboInputTask->ReadyForActivation();
}

void UGA_TwinSword_Combo::WaitForWeaponTraceEvents()
{
	const FRiftGameplayTags& RiftTags = FRiftGameplayTags::Get();

	UAbilityTask_WaitGameplayEvent* BeginTask =
		UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
			this,
			RiftTags.Event_Ability_TwinSword_Combo_WeaponTraceBegin,
			nullptr,
			false,
			true
		);
	BeginTask->EventReceived.AddDynamic(this, &UGA_TwinSword_Combo::HandleWeaponTraceBegin);
	BeginTask->ReadyForActivation();

	UAbilityTask_WaitGameplayEvent* TickTask =
		UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
			this,
			RiftTags.Event_Ability_TwinSword_Combo_WeaponTraceTick,
			nullptr,
			false,
			true
		);
	TickTask->EventReceived.AddDynamic(this, &UGA_TwinSword_Combo::HandleWeaponTraceTick);
	TickTask->ReadyForActivation();

	UAbilityTask_WaitGameplayEvent* EndTask =
		UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
			this,
			RiftTags.Event_Ability_TwinSword_Combo_WeaponTraceEnd,
			nullptr,
			false,
			true
		);
	EndTask->EventReceived.AddDynamic(this, &UGA_TwinSword_Combo::HandleWeaponTraceEnd);
	EndTask->ReadyForActivation();
}

void UGA_TwinSword_Combo::BufferInput()
{
	bHasBufferedInput = true;
}

void UGA_TwinSword_Combo::TryConsumeBufferedInput()
{
	if (!bHasBufferedInput) return;
	if (!bCanConsumeBufferedInput) return;

	bHasBufferedInput = false;
	bCanConsumeBufferedInput = false;
	JumpToNextComboSection();
}

void UGA_TwinSword_Combo::JumpToNextComboSection()
{
	UAnimInstance* AnimInstance = CurrentActorInfo
		? CurrentActorInfo->GetAnimInstance()
		: nullptr;

	if (!AnimInstance || !ActiveMontage) return;

	const int32 CurrentSectionIndex = GetCurrentComboSectionIndex();
	if (CurrentSectionIndex == INDEX_NONE) return;

	const int32 NextSectionIndex = CurrentSectionIndex + 1;
	if (!ActiveComboSections.IsValidIndex(NextSectionIndex)) return;

	const FName NextSection = ActiveComboSections[NextSectionIndex];

	if (CurrentActorInfo->IsNetAuthority())
	{
		if (UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo())
		{
			AbilitySystemComponent->CurrentMontageJumpToSection(NextSection);
		}
	}
	else if (CurrentActorInfo->IsLocallyControlled())
	{
		AnimInstance->Montage_JumpToSection(NextSection, ActiveMontage);
	}
}

int32 UGA_TwinSword_Combo::GetCurrentComboSectionIndex() const
{
	UAnimInstance* AnimInstance = CurrentActorInfo
		? CurrentActorInfo->GetAnimInstance()
		: nullptr;

	if (!AnimInstance || !ActiveMontage) return INDEX_NONE;

	const FName CurrentSection = AnimInstance->Montage_GetCurrentSection(ActiveMontage);
	return ActiveComboSections.IndexOfByKey(CurrentSection);
}

void UGA_TwinSword_Combo::BeginWeaponTrace()
{
	if (!CurrentActorInfo || !CurrentActorInfo->IsNetAuthority()) return;

	const APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(GetAvatarActorFromActorInfo());
	if (!PlayerCharacter) return;

	ResetWeaponTrace();

	for (APlayerWeapon* Weapon : PlayerCharacter->GetEquippedWeapons())
	{
		if (!Weapon) continue;

		FWeaponTraceState& TraceState = WeaponTraceStates.AddDefaulted_GetRef();
		TraceState.Weapon = Weapon;
		TraceState.PreviousStart = Weapon->GetTraceStartLocation();
		TraceState.PreviousEnd = Weapon->GetTraceEndLocation();
	}

	bWeaponTraceActive = !WeaponTraceStates.IsEmpty();
}

void UGA_TwinSword_Combo::PerformWeaponTrace()
{
	if (!bWeaponTraceActive || !CurrentActorInfo || !CurrentActorInfo->IsNetAuthority()) return;

	APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(GetAvatarActorFromActorInfo());
	const FPlayerAbilityEntry* ComboEntry = GetAbilityEntry();
	const URiftAbilityHitFragment* HitFragment = ComboEntry
		? ComboEntry->FindFragment<URiftAbilityHitFragment>()
		: nullptr;
	UWorld* World = GetWorld();

	if (!PlayerCharacter || !ComboEntry || !HitFragment || !World) return;

	constexpr int32 TraceSampleCount = 8;
	float TraceRadius = FMath::Max(1.0f, HitFragment->Radius);
	if (const UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo())
	{
		if (AbilitySystemComponent->HasMatchingGameplayTag(FRiftGameplayTags::Get().State_Ability_TwinSword_Enhance_Active))
		{
			TraceRadius *= FMath::Max(0.01f, HitFragment->EnhancedRadiusMultiplier);
		}
	}

	const FCollisionShape TraceShape = FCollisionShape::MakeSphere(TraceRadius);
	const FCollisionObjectQueryParams ObjectQueryParams(ECC_Pawn);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(TwinSwordWeaponTrace), false, PlayerCharacter);
	for (APlayerWeapon* Weapon : PlayerCharacter->GetEquippedWeapons())
	{
		QueryParams.AddIgnoredActor(Weapon);
	}

	for (FWeaponTraceState& TraceState : WeaponTraceStates)
	{
		APlayerWeapon* Weapon = TraceState.Weapon.Get();
		if (!Weapon) continue;

		const FVector CurrentStart = Weapon->GetTraceStartLocation();
		const FVector CurrentEnd = Weapon->GetTraceEndLocation();

		if (HitFragment->bDrawDebug)
		{
			DrawDebugLine(World, TraceState.PreviousStart, CurrentStart, FColor::Green, false, 0.2f, 0, 1.5f);
			DrawDebugLine(World, TraceState.PreviousEnd, CurrentEnd, FColor::Green, false, 0.2f, 0, 1.5f);
		}

		for (int32 SampleIndex = 0; SampleIndex < TraceSampleCount; ++SampleIndex)
		{
			const float Alpha = static_cast<float>(SampleIndex) / static_cast<float>(TraceSampleCount - 1);
			const FVector PreviousSample = FMath::Lerp(TraceState.PreviousStart, TraceState.PreviousEnd, Alpha);
			const FVector CurrentSample = FMath::Lerp(CurrentStart, CurrentEnd, Alpha);

			TArray<FHitResult> Hits;
			World->SweepMultiByObjectType(
				Hits,
				PreviousSample,
				CurrentSample,
				FQuat::Identity,
				ObjectQueryParams,
				TraceShape,
				QueryParams
			);

			if (HitFragment->bDrawDebug)
			{
				for (const FHitResult& Hit : Hits)
				{
					DrawDebugPoint(World, Hit.ImpactPoint, 12.0f, FColor::Yellow, false, 0.2f);
				}
			}

			for (const FHitResult& Hit : Hits)
			{
				AEnemyCharacter* HitEnemy = Cast<AEnemyCharacter>(Hit.GetActor());
				if (!HitEnemy) continue;

				const TWeakObjectPtr<AActor> HitActor(HitEnemy);
				if (HitActorsThisTraceWindow.Contains(HitActor)) continue;

				HitActorsThisTraceWindow.Add(HitActor);
				ApplyDamageToHitActor(HitEnemy, Hit);
				Logger::Log(
					PlayerCharacter,
					FString::Printf(TEXT("Weapon trace hit %s"), *GetNameSafe(HitEnemy)),
					ELogOutputType::LogOnly
				);
			}
		}

		TraceState.PreviousStart = CurrentStart;
		TraceState.PreviousEnd = CurrentEnd;
	}
}

void UGA_TwinSword_Combo::EndWeaponTrace()
{
	if (bWeaponTraceActive)
	{
		PerformWeaponTrace();
	}

	ResetWeaponTrace();
}

void UGA_TwinSword_Combo::ResetWeaponTrace()
{
	WeaponTraceStates.Reset();
	HitActorsThisTraceWindow.Reset();
	bWeaponTraceActive = false;
}

void UGA_TwinSword_Combo::ApplyDamageToHitActor(AActor* HitActor, const FHitResult& Hit)
{
	if (!CurrentActorInfo || !CurrentActorInfo->IsNetAuthority()) return;

	const FPlayerAbilityEntry* ComboEntry = GetAbilityEntry();
	const URiftAbilityComboFragment* ComboFragment = ComboEntry
		? ComboEntry->FindFragment<URiftAbilityComboFragment>()
		: nullptr;
	if (!ComboEntry || !ComboFragment) return;

	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
	if (!SourceASC) return;

	AEnemyCharacter* HitEnemy = Cast<AEnemyCharacter>(HitActor);
	if (!HitEnemy) return;

	UAbilitySystemComponent* TargetASC = HitEnemy->GetAbilitySystemComponent();
	if (!TargetASC) return;

	FGameplayEffectContextHandle EffectContext = SourceASC->MakeEffectContext();
	EffectContext.AddSourceObject(this);
	EffectContext.AddHitResult(Hit);

	FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(
		UGE_InstantDamage::StaticClass(),
		GetAbilityLevel(),
		EffectContext
	);

	if (!SpecHandle.IsValid()) return;

	const int32 ComboIndex = GetCurrentComboSectionIndex();
	if (!ComboFragment->Steps.IsValidIndex(ComboIndex)) return;

	float Damage = ComboFragment->Steps[ComboIndex].Damage;
	if (bCurrentAttackEmpowered)
	{
		Damage *= FMath::Max(0.0f, ComboFragment->EmpoweredDamageMultiplier);
	}

	SpecHandle.Data->SetSetByCallerMagnitude(
		FRiftGameplayTags::Get().Data_Damage,
		-Damage
	);

	SourceASC->ApplyGameplayEffectSpecToTarget(
		*SpecHandle.Data.Get(),
		TargetASC
	);

	const float NewHealth = TargetASC->GetNumericAttribute(UHealthAttributeSet::GetHealthAttribute());

	Logger::Log(
		this,
		FString::Printf(
			TEXT("Damaged %s, Health: %.0f"),
			*GetNameSafe(HitActor),
			NewHealth
		),
		ELogOutputType::LogOnly
	);
}

void UGA_TwinSword_Combo::HandleComboWindow(FGameplayEventData Payload)
{
	bCanConsumeBufferedInput = true;
	TryConsumeBufferedInput();
}

void UGA_TwinSword_Combo::HandleComboInputPressed(float TimeWaited)
{
	BufferInput();
	TryConsumeBufferedInput();
	WaitForComboInput();
}

void UGA_TwinSword_Combo::HandleWeaponTraceBegin(FGameplayEventData Payload)
{
	BeginWeaponTrace();
}

void UGA_TwinSword_Combo::HandleWeaponTraceTick(FGameplayEventData Payload)
{
	PerformWeaponTrace();
}

void UGA_TwinSword_Combo::HandleWeaponTraceEnd(FGameplayEventData Payload)
{
	EndWeaponTrace();
}

void UGA_TwinSword_Combo::HandleMontageCompleted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_TwinSword_Combo::HandleMontageCancelled()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UGA_TwinSword_Combo::HandleMontageInterrupted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}
