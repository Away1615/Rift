// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/GameplayAbilities/GA_TwinSword_MeleeComboBase.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Abilities/Tasks/AbilityTask_WaitInputPress.h"
#include "Animation/AnimInstance.h"
#include "Character/PlayerCharacter.h"
#include "Character/EnemyCharacter.h"
#include "AbilitySystem/GameplayEffects/GE_InstantDamage.h"
#include "Data/Player/PlayerClassConfig.h"
#include "Data/Player/Ability/AbilityDefinitionConfig.h"
#include "Data/Player/Ability/Fragments/AbilityMontageSectionsFragment.h"
#include "Data/Player/Ability/Fragments/AbilityEffectListFragment.h"
#include "Data/Player/Ability/Fragments/AbilityHitDetectionFragment.h"
#include "Data/Player/Ability/PlayerAbilitySetConfig.h"
#include "Debug/Logger.h"
#include "DrawDebugHelpers.h"
#include "AbilitySystem/Attributes/HealthAttributeSet.h"
#include "Equipment/PlayerWeapon.h"
#include "GameplayTags/RiftGameplayTags.h"

void UGA_TwinSword_MeleeComboBase::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
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

	const UAbilityDefinitionConfig* ComboDefinition = GetAbilityDefinition();
	const UAbilityMontageSectionsFragment* SectionsFragment = ComboDefinition
		? ComboDefinition->FindFragment<UAbilityMontageSectionsFragment>()
		: nullptr;
	if (!ComboDefinition || !SectionsFragment)
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

	if (ShouldCommitComboAbility() && !CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	SelectActiveComboSections(SectionsFragment);
	if (ActiveComboSections.IsEmpty())
	{
		Logger::Error(PlayerCharacter, TEXT("TwinSword active combo sections are missing"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	ComboActiveEffectHandle = ApplyInfiniteStateTagEffect(
		Handle,
		ActorInfo,
		ActivationInfo,
		GetAbilityActiveStateTag()
	);

	BufferedAttackInput = EAbilityInputID::None;
	bCanConsumeBufferedInput = true;

	WaitForComboInput();
	WaitForComboInputEvents();
	WaitForWeaponTraceEvents();
	if (!PlayComboSection(0))
	{
		Logger::Error(PlayerCharacter, TEXT("TwinSword first combo section montage is missing"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
	}
}

void UGA_TwinSword_MeleeComboBase::EndAbility(const FGameplayAbilitySpecHandle Handle,
                                     const FGameplayAbilityActorInfo* ActorInfo,
                                     const FGameplayAbilityActivationInfo ActivationInfo,
                                     bool bReplicateEndAbility,
                                     bool bWasCancelled)
{
	RemoveGrantedStateTagEffect(ComboActiveEffectHandle);

	ActiveMontage = nullptr;
	ComboMontage = nullptr;
	ActiveComboSections.Empty();
	ActiveSectionIndex = INDEX_NONE;
	BufferedAttackInput = EAbilityInputID::None;
	bCanConsumeBufferedInput = false;
	ResetWeaponTrace();

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

FGameplayTag UGA_TwinSword_MeleeComboBase::GetAbilityActiveStateTag() const
{
	return FGameplayTag();
}

bool UGA_TwinSword_MeleeComboBase::ShouldCommitComboAbility() const
{
	return true;
}

bool UGA_TwinSword_MeleeComboBase::CheckCost(const FGameplayAbilitySpecHandle Handle,
                                    const FGameplayAbilityActorInfo* ActorInfo,
                                    FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!ShouldCommitComboAbility())
	{
		return Super::CheckCost(Handle, ActorInfo, OptionalRelevantTags);
	}

	return Super::CheckCost(Handle, ActorInfo, OptionalRelevantTags);
}

void UGA_TwinSword_MeleeComboBase::ApplyCost(const FGameplayAbilitySpecHandle Handle,
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

void UGA_TwinSword_MeleeComboBase::WaitForCurrentSectionChainPoint()
{
	if (!ActiveComboSections.IsValidIndex(ActiveSectionIndex)) return;

	const FGameplayTag ChainPointEventTag = ActiveComboSections[ActiveSectionIndex].ChainPointEventTag;
	if (!ChainPointEventTag.IsValid()) return;

	UAbilityTask_WaitGameplayEvent* ChainPointTask =
		UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
			this,
			ChainPointEventTag,
			nullptr,
			true,
			true
		);

	ChainPointTask->EventReceived.AddDynamic(
		this,
		&UGA_TwinSword_MeleeComboBase::HandleComboChainPoint
	);

	ChainPointTask->ReadyForActivation();
}

void UGA_TwinSword_MeleeComboBase::WaitForComboInput()
{
	UAbilityTask_WaitInputPress* ComboInputTask =
		UAbilityTask_WaitInputPress::WaitInputPress(this, false);

	ComboInputTask->OnPress.AddDynamic(
		this,
		&UGA_TwinSword_MeleeComboBase::HandleComboInputPressed
	);

	ComboInputTask->ReadyForActivation();
}

void UGA_TwinSword_MeleeComboBase::WaitForComboInputEvents()
{
	const FRiftGameplayTags& RiftTags = FRiftGameplayTags::Get();

	UAbilityTask_WaitGameplayEvent* PrimaryInputTask =
		UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
			this,
			RiftTags.Event_TwinSword_Combo_Input_Primary,
			nullptr,
			false,
			true
		);
	PrimaryInputTask->EventReceived.AddDynamic(this, &UGA_TwinSword_MeleeComboBase::HandlePrimaryComboInputEvent);
	PrimaryInputTask->ReadyForActivation();

	UAbilityTask_WaitGameplayEvent* SecondaryInputTask =
		UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
			this,
			RiftTags.Event_TwinSword_Combo_Input_Secondary,
			nullptr,
			false,
			true
		);
	SecondaryInputTask->EventReceived.AddDynamic(this, &UGA_TwinSword_MeleeComboBase::HandleSecondaryComboInputEvent);
	SecondaryInputTask->ReadyForActivation();
}

void UGA_TwinSword_MeleeComboBase::WaitForWeaponTraceEvents()
{
	const FRiftGameplayTags& RiftTags = FRiftGameplayTags::Get();

	UAbilityTask_WaitGameplayEvent* BeginTask =
		UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
			this,
			RiftTags.Event_TwinSword_Combo_WeaponTrace_Begin,
			nullptr,
			false,
			true
		);
	BeginTask->EventReceived.AddDynamic(this, &UGA_TwinSword_MeleeComboBase::HandleWeaponTraceBegin);
	BeginTask->ReadyForActivation();

	UAbilityTask_WaitGameplayEvent* TickTask =
		UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
			this,
			RiftTags.Event_TwinSword_Combo_WeaponTrace_Tick,
			nullptr,
			false,
			true
		);
	TickTask->EventReceived.AddDynamic(this, &UGA_TwinSword_MeleeComboBase::HandleWeaponTraceTick);
	TickTask->ReadyForActivation();

	UAbilityTask_WaitGameplayEvent* EndTask =
		UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
			this,
			RiftTags.Event_TwinSword_Combo_WeaponTrace_End,
			nullptr,
			false,
			true
		);
	EndTask->EventReceived.AddDynamic(this, &UGA_TwinSword_MeleeComboBase::HandleWeaponTraceEnd);
	EndTask->ReadyForActivation();
}

void UGA_TwinSword_MeleeComboBase::BufferInput(const EAbilityInputID InputID)
{
	BufferedAttackInput = InputID;
}

void UGA_TwinSword_MeleeComboBase::ConsumeBufferedInputAtChainPoint()
{
	if (BufferedAttackInput == EAbilityInputID::None) return;

	const EAbilityInputID InputToConsume = BufferedAttackInput;
	if (InputToConsume == GetCurrentAbilityInputID())
	{
		TryAdvanceComboSection();
		return;
	}

	if (!TryActivateBufferedComboAbility(InputToConsume))
	{
		BufferedAttackInput = EAbilityInputID::None;
		bCanConsumeBufferedInput = false;
	}
}

void UGA_TwinSword_MeleeComboBase::TryAdvanceComboSection()
{
	const int32 NextSectionIndex = ActiveSectionIndex + 1;
	if (!ActiveComboSections.IsValidIndex(NextSectionIndex)) return;

	EndWeaponTrace(INDEX_NONE);

	BufferedAttackInput = EAbilityInputID::None;
	bCanConsumeBufferedInput = false;
	if (!PlayComboSection(NextSectionIndex))
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
	}
}

bool UGA_TwinSword_MeleeComboBase::TryActivateBufferedComboAbility(const EAbilityInputID InputID)
{
	UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo();
	if (!AbilitySystemComponent)
	{
		return false;
	}

	FGameplayAbilitySpecHandle TargetHandle;
	if (!TryFindAbilityHandleForInput(InputID, TargetHandle))
	{
		return false;
	}

	EndWeaponTrace(INDEX_NONE);
	BufferedAttackInput = EAbilityInputID::None;
	bCanConsumeBufferedInput = false;
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	return AbilitySystemComponent->TryActivateAbility(TargetHandle);
}

bool UGA_TwinSword_MeleeComboBase::TryFindAbilityHandleForInput(
	const EAbilityInputID InputID,
	FGameplayAbilitySpecHandle& OutHandle) const
{
	const UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo();
	if (!AbilitySystemComponent)
	{
		return false;
	}

	const int32 InputIDValue = static_cast<int32>(InputID);
	for (const FGameplayAbilitySpec& AbilitySpec : AbilitySystemComponent->GetActivatableAbilities())
	{
		if (AbilitySpec.InputID == InputIDValue)
		{
			OutHandle = AbilitySpec.Handle;
			return true;
		}
	}

	return false;
}

EAbilityInputID UGA_TwinSword_MeleeComboBase::GetCurrentAbilityInputID() const
{
	const UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo();
	const FGameplayAbilitySpec* AbilitySpec = AbilitySystemComponent
		? AbilitySystemComponent->FindAbilitySpecFromHandle(CurrentSpecHandle)
		: nullptr;

	return AbilitySpec
		? static_cast<EAbilityInputID>(AbilitySpec->InputID)
		: EAbilityInputID::None;
}

bool UGA_TwinSword_MeleeComboBase::PlayComboSection(const int32 SectionIndex)
{
	if (!ActiveComboSections.IsValidIndex(SectionIndex)) return false;

	const FAbilityMontageSection& Section = ActiveComboSections[SectionIndex];
	UAnimMontage* MontageToPlay = ResolveSectionMontage(Section);
	if (!MontageToPlay) return false;

	ActiveSectionIndex = SectionIndex;
	BufferedAttackInput = EAbilityInputID::None;
	bCanConsumeBufferedInput = true;
	WaitForCurrentSectionChainPoint();

	const float PlayRate = Section.PlayRate * GetActiveAttackSpeedMultiplier();
	const FName SectionName = ResolveSectionName(Section);
	UAnimInstance* AnimInstance = CurrentActorInfo
		? CurrentActorInfo->GetAnimInstance()
		: nullptr;

	if (ActiveMontage == MontageToPlay && AnimInstance)
	{
		AnimInstance->Montage_SetPlayRate(ActiveMontage, PlayRate);
		if (!SectionName.IsNone())
		{
			AnimInstance->Montage_JumpToSection(SectionName, ActiveMontage);
		}
		return true;
	}

	ActiveMontage = MontageToPlay;
	return TryPlayMontage(CurrentSpecHandle, CurrentActorInfo, MontageToPlay, PlayRate, SectionName, true);
}

UAnimMontage* UGA_TwinSword_MeleeComboBase::ResolveSectionMontage(const FAbilityMontageSection& Section) const
{
	return ComboMontage ? ComboMontage.Get() : Section.Montage.Get();
}

FName UGA_TwinSword_MeleeComboBase::ResolveSectionName(const FAbilityMontageSection& Section) const
{
	return Section.SectionName;
}

bool UGA_TwinSword_MeleeComboBase::IsCurrentSectionWaitingForChainPoint() const
{
	return ActiveComboSections.IsValidIndex(ActiveSectionIndex)
		&& ActiveComboSections[ActiveSectionIndex].ChainPointEventTag.IsValid();
}

const UAbilityMontageSectionsFragment* UGA_TwinSword_MeleeComboBase::GetSectionsFragment() const
{
	const UAbilityDefinitionConfig* ComboDefinition = GetAbilityDefinition();
	return ComboDefinition
		? ComboDefinition->FindFragment<UAbilityMontageSectionsFragment>()
		: nullptr;
}

void UGA_TwinSword_MeleeComboBase::SelectActiveComboSections(const UAbilityMontageSectionsFragment* SectionsFragment)
{
	ActiveComboSections.Reset();
	ComboMontage = SectionsFragment ? SectionsFragment->Montage.Get() : nullptr;
	if (SectionsFragment) ActiveComboSections = SectionsFragment->Sections;
}

void UGA_TwinSword_MeleeComboBase::InitWeaponTraceStates()
{
	if (!WeaponTraceStates.IsEmpty()) return;

	const APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(GetAvatarActorFromActorInfo());
	if (!PlayerCharacter) return;

	for (APlayerWeapon* Weapon : PlayerCharacter->GetEquippedWeapons())
	{
		if (!Weapon) continue;
		FWeaponTraceState& State = WeaponTraceStates.AddDefaulted_GetRef();
		State.Weapon = Weapon;
	}
}

void UGA_TwinSword_MeleeComboBase::BeginWeaponTrace(const int32 WeaponIndex)
{
	if (!CurrentActorInfo || !CurrentActorInfo->IsNetAuthority()) return;

	InitWeaponTraceStates();

	for (int32 i = 0; i < WeaponTraceStates.Num(); ++i)
	{
		if (WeaponIndex != INDEX_NONE && i != WeaponIndex) continue;

		FWeaponTraceState& State = WeaponTraceStates[i];
		APlayerWeapon* Weapon = State.Weapon.Get();
		if (!Weapon) continue;

		State.bTraceActive = true;
		State.HitActors.Reset();
		State.PreviousStart = Weapon->GetTraceStartLocation();
		State.PreviousEnd = Weapon->GetTraceEndLocation();
	}
}

void UGA_TwinSword_MeleeComboBase::PerformWeaponTrace(const int32 WeaponIndex)
{
	if (!CurrentActorInfo || !CurrentActorInfo->IsNetAuthority()) return;

	APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(GetAvatarActorFromActorInfo());
	const UAbilityDefinitionConfig* ComboDefinition = GetAbilityDefinition();
	const UAbilityHitDetectionFragment* HitDetectionFragment = ComboDefinition
		? ComboDefinition->FindFragment<UAbilityHitDetectionFragment>()
		: nullptr;
	UWorld* World = GetWorld();

	if (!PlayerCharacter || !ComboDefinition || !HitDetectionFragment || !World) return;

	constexpr int32 TraceSampleCount = 8;
	float TraceRadius = FMath::Max(1.0f, HitDetectionFragment->Radius) * GetActiveAttackRangeMultiplier();
	if (const UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo())
	{
		const FRiftGameplayTags& RiftTags = FRiftGameplayTags::Get();
		if (AbilitySystemComponent->HasMatchingGameplayTag(RiftTags.State_TwinSword_ArmageddonBlade_Active))
		{
			TraceRadius *= FMath::Max(0.01f, HitDetectionFragment->EnhancedRadiusMultiplier);
		}
	}

	const FCollisionShape TraceShape = FCollisionShape::MakeSphere(TraceRadius);
	const FCollisionObjectQueryParams ObjectQueryParams(ECC_Pawn);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(TwinSwordWeaponTrace), false, PlayerCharacter);
	for (APlayerWeapon* Weapon : PlayerCharacter->GetEquippedWeapons())
	{
		QueryParams.AddIgnoredActor(Weapon);
	}

	for (int32 i = 0; i < WeaponTraceStates.Num(); ++i)
	{
		if (WeaponIndex != INDEX_NONE && i != WeaponIndex) continue;

		FWeaponTraceState& TraceState = WeaponTraceStates[i];
		if (!TraceState.bTraceActive) continue;

		APlayerWeapon* Weapon = TraceState.Weapon.Get();
		if (!Weapon) continue;

		const FVector CurrentStart = Weapon->GetTraceStartLocation();
		const FVector CurrentEnd = Weapon->GetTraceEndLocation();

		if (HitDetectionFragment->bDrawDebug)
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

			if (HitDetectionFragment->bDrawDebug)
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
				if (TraceState.HitActors.Contains(HitActor)) continue;

				TraceState.HitActors.Add(HitActor);
				ApplyDamageToHitActor(HitEnemy, Hit);
				Logger::Log(
					PlayerCharacter,
					FString::Printf(TEXT("Weapon[%d] trace hit %s"), i, *GetNameSafe(HitEnemy)),
					ELogOutputType::LogOnly
				);
			}
		}

		TraceState.PreviousStart = CurrentStart;
		TraceState.PreviousEnd = CurrentEnd;
	}
}

void UGA_TwinSword_MeleeComboBase::EndWeaponTrace(const int32 WeaponIndex)
{
	PerformWeaponTrace(WeaponIndex);

	for (int32 i = 0; i < WeaponTraceStates.Num(); ++i)
	{
		if (WeaponIndex != INDEX_NONE && i != WeaponIndex) continue;
		FWeaponTraceState& State = WeaponTraceStates[i];
		State.bTraceActive = false;
		State.HitActors.Reset();
	}
}

void UGA_TwinSword_MeleeComboBase::ResetWeaponTrace()
{
	WeaponTraceStates.Reset();
}

void UGA_TwinSword_MeleeComboBase::ApplyDamageToHitActor(AActor* HitActor, const FHitResult& Hit)
{
	if (!CurrentActorInfo || !CurrentActorInfo->IsNetAuthority()) return;

	if (!ActiveComboSections.IsValidIndex(ActiveSectionIndex)) return;

	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
	if (!SourceASC) return;

	AEnemyCharacter* HitEnemy = Cast<AEnemyCharacter>(HitActor);
	if (!HitEnemy) return;

	UAbilitySystemComponent* TargetASC = HitEnemy->GetAbilitySystemComponent();
	if (!TargetASC) return;

	ApplyHitEffects(HitActor, Hit, ActiveComboSections[ActiveSectionIndex].Effects);

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

void UGA_TwinSword_MeleeComboBase::ApplyHitEffects(
	AActor* HitActor,
	const FHitResult& Hit,
	const TArray<FAbilityEffectConfig>& StepEffects) const
{
	TArray<FAbilityEffectConfig> Effects = StepEffects;
	CollectAbilityEffects(Effects);

	float DamageMultiplier = 1.0f;
	for (const FAbilityEffectConfig& Effect : Effects)
	{
		if (Effect.Trigger != EAbilityEffectTriggerEvent::OnHit) continue;
		if (Effect.Type != EAbilityEffectKind::DamageMultiplier) continue;
		if (!DoesEffectPassCondition(Effect)) continue;
		if (Effect.Magnitude <= 0.0f) continue;

		DamageMultiplier *= Effect.Magnitude;
	}

	for (const FAbilityEffectConfig& Effect : Effects)
	{
		if (Effect.Trigger != EAbilityEffectTriggerEvent::OnHit) continue;
		if (!DoesEffectPassCondition(Effect)) continue;

		switch (Effect.Type)
		{
		case EAbilityEffectKind::Damage:
			ApplyHealthEffect(HitActor, Hit, -Effect.Magnitude * DamageMultiplier);
			break;
		case EAbilityEffectKind::PhantomDamage:
			ApplyHealthEffect(HitActor, Hit, -Effect.Magnitude);
			break;
		case EAbilityEffectKind::Heal:
			ApplyHealthEffect(GetAvatarActorFromActorInfo(), Hit, Effect.Magnitude);
			break;
		case EAbilityEffectKind::CooldownReduction:
			ApplyCooldownReduction(Effect);
			break;
		case EAbilityEffectKind::SwordWave:
			ApplySwordWave(Effect);
			break;
		default:
			break;
		}
	}
}

void UGA_TwinSword_MeleeComboBase::ApplyHealthEffect(AActor* TargetActor, const FHitResult& Hit, const float Magnitude) const
{
	if (FMath::IsNearlyZero(Magnitude)) return;

	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
	if (!SourceASC || !TargetActor) return;

	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
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

	SpecHandle.Data->SetSetByCallerMagnitude(
		FRiftGameplayTags::Get().Data_Damage,
		Magnitude
	);

	SourceASC->ApplyGameplayEffectSpecToTarget(
		*SpecHandle.Data.Get(),
		TargetASC
	);
}

void UGA_TwinSword_MeleeComboBase::ApplySwordWave(const FAbilityEffectConfig& Effect) const
{
	if (!CurrentActorInfo || !CurrentActorInfo->IsNetAuthority()) return;
	if (Effect.Magnitude <= 0.0f) return;

	AActor* SourceActor = GetAvatarActorFromActorInfo();
	UWorld* World = GetWorld();
	if (!SourceActor || !World) return;

	const FVector Forward = SourceActor->GetActorForwardVector();
	const FVector Start = SourceActor->GetActorLocation() + Forward * 50.0f;
	const FVector End = Start + Forward * FMath::Max(100.0f, Effect.Range);
	const float Radius = FMath::Max(1.0f, Effect.Radius);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(TwinSwordSwordWave), false, SourceActor);
	const FCollisionShape TraceShape = FCollisionShape::MakeSphere(Radius);
	const FCollisionObjectQueryParams ObjectQueryParams(ECC_Pawn);

	TArray<FHitResult> Hits;
	World->SweepMultiByObjectType(
		Hits,
		Start,
		End,
		FQuat::Identity,
		ObjectQueryParams,
		TraceShape,
		QueryParams
	);

	if (Effect.bDrawDebug)
	{
		DrawDebugLine(World, Start, End, FColor::Blue, false, 0.35f, 0, 2.0f);
		DrawDebugSphere(World, End, Radius, 16, FColor::Blue, false, 0.35f);
	}

	TSet<TWeakObjectPtr<AActor>> DamagedActors;
	for (const FHitResult& WaveHit : Hits)
	{
		AEnemyCharacter* Enemy = Cast<AEnemyCharacter>(WaveHit.GetActor());
		if (!Enemy) continue;

		const TWeakObjectPtr<AActor> DamagedActor(Enemy);
		if (DamagedActors.Contains(DamagedActor)) continue;

		DamagedActors.Add(DamagedActor);
		ApplyHealthEffect(Enemy, WaveHit, -Effect.Magnitude);
	}
}

void UGA_TwinSword_MeleeComboBase::ApplyCooldownReduction(const FAbilityEffectConfig& Effect) const
{
	if (!CurrentActorInfo || !CurrentActorInfo->IsNetAuthority()) return;
	if (Effect.Magnitude <= 0.0f || Effect.CooldownTags.IsEmpty()) return;

	UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo();
	if (!AbilitySystemComponent) return;

	const FGameplayEffectQuery Query = FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(Effect.CooldownTags);
	const TArray<FActiveGameplayEffectHandle> Handles = AbilitySystemComponent->GetActiveEffects(Query);
	const TArray<TPair<float, float>> Times = AbilitySystemComponent->GetActiveEffectsTimeRemainingAndDuration(Query);
	const int32 Count = FMath::Min(Handles.Num(), Times.Num());

	for (int32 Index = 0; Index < Count; ++Index)
	{
		const float TimeRemaining = Times[Index].Key;
		if (TimeRemaining <= Effect.Magnitude)
		{
			AbilitySystemComponent->RemoveActiveGameplayEffect(Handles[Index]);
		}
	}
}

void UGA_TwinSword_MeleeComboBase::CollectAbilityEffects(TArray<FAbilityEffectConfig>& OutEffects) const
{
	const UAbilityDefinitionConfig* ComboDefinition = GetAbilityDefinition();
	if (const UAbilityEffectListFragment* EffectListFragment = ComboDefinition
		? ComboDefinition->FindFragment<UAbilityEffectListFragment>()
		: nullptr)
	{
		OutEffects.Append(EffectListFragment->Effects);
	}

	const APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(GetAvatarActorFromActorInfo());
	const UPlayerAbilitySetConfig* AbilitySetConfig =
		PlayerCharacter && PlayerCharacter->GetPlayerClassConfig()
			? PlayerCharacter->GetPlayerClassConfig()->PlayerAbilityConfig
			: nullptr;
	const UAbilityDefinitionConfig* PassiveAbility = AbilitySetConfig
		? AbilitySetConfig->FindAbilityBySlot(EAbilitySlot::Passive)
		: nullptr;
	if (const UAbilityEffectListFragment* PassiveEffects = PassiveAbility
		? PassiveAbility->FindFragment<UAbilityEffectListFragment>()
		: nullptr)
	{
		OutEffects.Append(PassiveEffects->Effects);
	}
}

bool UGA_TwinSword_MeleeComboBase::DoesEffectPassCondition(const FAbilityEffectConfig& Effect) const
{
	if (!Effect.RequiredOwnerTag.IsValid())
	{
		return true;
	}

	const UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo();
	return AbilitySystemComponent && AbilitySystemComponent->HasMatchingGameplayTag(Effect.RequiredOwnerTag);
}

float UGA_TwinSword_MeleeComboBase::GetActiveDamageMultiplier() const
{
	TArray<FAbilityEffectConfig> Effects;
	CollectAbilityEffects(Effects);

	float Multiplier = 1.0f;
	for (const FAbilityEffectConfig& Effect : Effects)
	{
		if (Effect.Type != EAbilityEffectKind::DamageMultiplier) continue;
		if (!DoesEffectPassCondition(Effect)) continue;
		if (Effect.Magnitude <= 0.0f) continue;

		Multiplier *= Effect.Magnitude;
	}

	return Multiplier;
}

float UGA_TwinSword_MeleeComboBase::GetActiveAttackSpeedMultiplier() const
{
	TArray<FAbilityEffectConfig> Effects;
	CollectAbilityEffects(Effects);

	float Multiplier = 1.0f;
	for (const FAbilityEffectConfig& Effect : Effects)
	{
		if (Effect.Type != EAbilityEffectKind::AttackSpeedMultiplier) continue;
		if (!DoesEffectPassCondition(Effect)) continue;
		if (Effect.Magnitude <= 0.0f) continue;

		Multiplier *= Effect.Magnitude;
	}

	return Multiplier;
}

float UGA_TwinSword_MeleeComboBase::GetActiveAttackRangeMultiplier() const
{
	TArray<FAbilityEffectConfig> Effects;
	CollectAbilityEffects(Effects);

	float Multiplier = 1.0f;
	for (const FAbilityEffectConfig& Effect : Effects)
	{
		if (Effect.Type != EAbilityEffectKind::AttackRangeMultiplier) continue;
		if (!DoesEffectPassCondition(Effect)) continue;
		if (Effect.Magnitude <= 0.0f) continue;

		Multiplier *= Effect.Magnitude;
	}

	return Multiplier;
}

void UGA_TwinSword_MeleeComboBase::HandleComboChainPoint(FGameplayEventData Payload)
{

	Logger::Log(this, FString::Printf(TEXT("ChainPoint at section %d. BufferedInput=%d"),
		ActiveSectionIndex, static_cast<int32>(BufferedAttackInput)));
	if (BufferedAttackInput == EAbilityInputID::None || !bCanConsumeBufferedInput)
	{
		bCanConsumeBufferedInput = false;
		return;
	}

	ConsumeBufferedInputAtChainPoint();
}

void UGA_TwinSword_MeleeComboBase::HandleComboInputPressed(float TimeWaited)
{
	Logger::Log(this, FString::Printf(TEXT("Input pressed at section %d"), ActiveSectionIndex));
	BufferInput(GetCurrentAbilityInputID());
	WaitForComboInput();
}

void UGA_TwinSword_MeleeComboBase::HandlePrimaryComboInputEvent(FGameplayEventData Payload)
{
	Logger::Log(this, FString::Printf(TEXT("Primary input buffered at section %d"), ActiveSectionIndex));
	BufferInput(EAbilityInputID::Primary);
}

void UGA_TwinSword_MeleeComboBase::HandleSecondaryComboInputEvent(FGameplayEventData Payload)
{
	Logger::Log(this, FString::Printf(TEXT("Secondary input buffered at section %d"), ActiveSectionIndex));
	BufferInput(EAbilityInputID::Secondary);
}

void UGA_TwinSword_MeleeComboBase::HandleWeaponTraceBegin(FGameplayEventData Payload)
{
	BeginWeaponTrace(static_cast<int32>(Payload.EventMagnitude));
}

void UGA_TwinSword_MeleeComboBase::HandleWeaponTraceTick(FGameplayEventData Payload)
{
	PerformWeaponTrace(static_cast<int32>(Payload.EventMagnitude));
}

void UGA_TwinSword_MeleeComboBase::HandleWeaponTraceEnd(FGameplayEventData Payload)
{
	EndWeaponTrace(static_cast<int32>(Payload.EventMagnitude));
}

bool UGA_TwinSword_MeleeComboBase::OnAbilityMontageCancelled()
{
	return true;
}

bool UGA_TwinSword_MeleeComboBase::OnAbilityMontageInterrupted()
{
	return true;
}
