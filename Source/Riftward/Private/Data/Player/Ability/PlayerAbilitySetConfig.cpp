// Fill out your copyright notice in the Description page of Project Settings.

#include "Data/Player/Ability/PlayerAbilitySetConfig.h"

#include "AbilitySystem/GameplayAbilities/GA_TwinSword_ConfigDrivenAction.h"
#include "AbilitySystem/GameplayAbilities/GA_TwinSword_Dodge.h"
#include "AbilitySystem/GameplayAbilities/GA_TwinSword_PrimaryCombo.h"
#include "AbilitySystem/GameplayAbilities/GA_TwinSword_SecondaryCombo.h"
#include "Animation/AnimMontage.h"
#include "Data/Player/Ability/BaseAbilityConfig.h"
#include "Data/Player/Ability/TwinSword/TwinSwordComboAbilityConfig.h"
#include "Data/Player/Ability/TwinSword/TwinSwordCoreAbilityConfig.h"
#include "Data/Player/Ability/Fragments/AbilityActionExecutionFragment.h"
#include "Data/Player/Ability/Fragments/AbilityTimedStateFragment.h"
#include "Data/Player/Ability/Fragments/AbilityMontageSectionsFragment.h"
#include "Data/Player/Ability/Fragments/AbilityResourceCostFragment.h"
#include "Data/Player/Ability/Fragments/AbilityPerfectDodgeFragment.h"
#include "Data/Player/Ability/Fragments/AbilityEffectListFragment.h"
#include "Data/Player/Ability/Fragments/AbilityHitDetectionFragment.h"
#include "GameplayTags/RiftGameplayTags.h"

namespace
{
	TSubclassOf<UBaseGameplayAbility> ResolveDeprecatedAbilityClass(const FDeprecatedPlayerAbilityEntry& AbilityEntry)
	{
		if (AbilityEntry.AbilityClass)
		{
			return AbilityEntry.AbilityClass;
		}

		switch (AbilityEntry.AbilityType)
		{
		case EAbilityType::TwinSwordCombo:
			return AbilityEntry.Slot == EAbilitySlot::Secondary
				? UGA_TwinSword_SecondaryCombo::StaticClass()
				: UGA_TwinSword_PrimaryCombo::StaticClass();
		case EAbilityType::TwinSwordDodge:
			return UGA_TwinSword_Dodge::StaticClass();
		case EAbilityType::TwinSwordSignature:
		case EAbilityType::TwinSwordEnhance:
			return UGA_TwinSword_ConfigDrivenAction::StaticClass();
		default:
			return nullptr;
		}
	}

	FGameplayTag ResolveDeprecatedAbilityID(const FDeprecatedPlayerAbilityEntry& AbilityEntry)
	{
		const FRiftGameplayTags& Tags = FRiftGameplayTags::Get();

		if (AbilityEntry.AbilityType == EAbilityType::TwinSwordDodge || AbilityEntry.Slot == EAbilitySlot::Core)
		{
			return Tags.Ability_TwinSword_Dodge;
		}

		if (AbilityEntry.AbilityType == EAbilityType::TwinSwordCombo)
		{
			return AbilityEntry.Slot == EAbilitySlot::Secondary
				? Tags.Ability_TwinSword_SecondaryCombo
				: Tags.Ability_TwinSword_PrimaryCombo;
		}

		if (AbilityEntry.AbilityType == EAbilityType::TwinSwordSignature || AbilityEntry.Slot == EAbilitySlot::Signature)
		{
			return Tags.Ability_TwinSword_ArmageddonBlade;
		}

		if (AbilityEntry.AbilityType == EAbilityType::TwinSwordEnhance || AbilityEntry.Slot == EAbilitySlot::Enhance)
		{
			return Tags.Ability_TwinSword_ArmageddonBlade;
		}

		if (AbilityEntry.Slot == EAbilitySlot::Ultimate)
		{
			return Tags.Ability_TwinSword_Ultimate;
		}

		return AbilityEntry.AbilityID;
	}

	void DuplicateFragments(
		UObject* NewOuter,
		const TArray<TObjectPtr<UAbilityConfigFragment>>& SourceFragments,
		TArray<TObjectPtr<UAbilityConfigFragment>>& TargetFragments)
	{
		TargetFragments.Reset();

		for (const TObjectPtr<UAbilityConfigFragment>& SourceFragment : SourceFragments)
		{
			if (!SourceFragment) continue;

			UAbilityConfigFragment* FragmentCopy = DuplicateObject<UAbilityConfigFragment>(SourceFragment, NewOuter);
			if (FragmentCopy)
			{
				TargetFragments.Add(FragmentCopy);
			}
		}
	}
}

UPlayerAbilitySetConfig::UPlayerAbilitySetConfig()
{
}

void UPlayerAbilitySetConfig::PostLoad()
{
	Super::PostLoad();

	if (!HasAnyConfiguredAbility() && !Abilities.IsEmpty())
	{
		ApplyDeprecatedAbilityEntries();
	}

	ApplyDeprecatedConfigData();
}

void UPlayerAbilitySetConfig::GetGrantableAbilities(TArray<FPlayerAbilityGrant>& OutAbilities) const
{
	OutAbilities.Reset();

	auto AddGrant = [&OutAbilities](EAbilitySlot Slot, const FPlayerAbilitySlotConfig& SlotConfig)
	{
		if (!SlotConfig.Ability) return;

		FPlayerAbilityGrant& Grant = OutAbilities.AddDefaulted_GetRef();
		Grant.Slot = Slot;
		Grant.Ability = SlotConfig.Ability;
		Grant.bAutoGrant = SlotConfig.bAutoGrant;
	};

	AddGrant(EAbilitySlot::Passive, PassiveAbility);
	AddGrant(EAbilitySlot::Primary, PrimaryAbility);
	AddGrant(EAbilitySlot::PrimaryHeavy, PrimaryHeavyAbility);
	AddGrant(EAbilitySlot::Secondary, SecondaryAbility);
	AddGrant(EAbilitySlot::SecondaryHeavy, SecondaryHeavyAbility);
	AddGrant(EAbilitySlot::Core, CoreAbility);
	AddGrant(EAbilitySlot::Special, SpecialAbility);
	AddGrant(EAbilitySlot::Enhance, EnhanceAbility);
	AddGrant(EAbilitySlot::Ultimate, UltimateAbility);
}

const UAbilityDefinitionConfig* UPlayerAbilitySetConfig::FindAbilityByID(FGameplayTag AbilityID) const
{
	if (!AbilityID.IsValid()) return nullptr;

	TArray<FPlayerAbilityGrant> Grants;
	GetGrantableAbilities(Grants);

	for (const FPlayerAbilityGrant& Grant : Grants)
	{
		if (Grant.Ability && Grant.Ability->AbilityID == AbilityID)
		{
			return Grant.Ability;
		}
	}

	return nullptr;
}

const UAbilityDefinitionConfig* UPlayerAbilitySetConfig::FindAbilityByInputID(EAbilityInputID InputID) const
{
	if (InputID == EAbilityInputID::None) return nullptr;
	return FindAbilityBySlot(GetSlotForInputID(InputID));
}

const UAbilityDefinitionConfig* UPlayerAbilitySetConfig::FindAbilityBySlot(EAbilitySlot Slot) const
{
	const FPlayerAbilitySlotConfig* SlotConfig = FindSlotConfig(Slot);
	return SlotConfig ? SlotConfig->Ability : nullptr;
}

EAbilityInputID UPlayerAbilitySetConfig::GetInputIDForSlot(EAbilitySlot Slot)
{
	switch (Slot)
	{
	case EAbilitySlot::Primary:
		return EAbilityInputID::Primary;
	case EAbilitySlot::PrimaryHeavy:
		return EAbilityInputID::PrimaryHeavy;
	case EAbilitySlot::Secondary:
		return EAbilityInputID::Secondary;
	case EAbilitySlot::SecondaryHeavy:
		return EAbilityInputID::SecondaryHeavy;
	case EAbilitySlot::Core:
		return EAbilityInputID::Core;
	case EAbilitySlot::Special:
		return EAbilityInputID::Special;
	case EAbilitySlot::Signature:
		return EAbilityInputID::Signature;
	case EAbilitySlot::Enhance:
		return EAbilityInputID::Enhance;
	case EAbilitySlot::Ultimate:
		return EAbilityInputID::Ultimate;
	case EAbilitySlot::Passive:
	default:
		return EAbilityInputID::None;
	}
}

EAbilitySlot UPlayerAbilitySetConfig::GetSlotForInputID(EAbilityInputID InputID)
{
	switch (InputID)
	{
	case EAbilityInputID::Primary:
		return EAbilitySlot::Primary;
	case EAbilityInputID::PrimaryHeavy:
		return EAbilitySlot::PrimaryHeavy;
	case EAbilityInputID::Secondary:
		return EAbilitySlot::Secondary;
	case EAbilityInputID::SecondaryHeavy:
		return EAbilitySlot::SecondaryHeavy;
	case EAbilityInputID::Core:
		return EAbilitySlot::Core;
	case EAbilityInputID::Special:
		return EAbilitySlot::Special;
	case EAbilityInputID::Signature:
		return EAbilitySlot::Signature;
	case EAbilityInputID::Enhance:
		return EAbilitySlot::Enhance;
	case EAbilityInputID::Ultimate:
		return EAbilitySlot::Ultimate;
	default:
		return EAbilitySlot::Passive;
	}
}

bool UPlayerAbilitySetConfig::HasAnyConfiguredAbility() const
{
	return PassiveAbility.Ability
		|| PrimaryAbility.Ability
		|| PrimaryHeavyAbility.Ability
		|| SecondaryAbility.Ability
		|| SecondaryHeavyAbility.Ability
		|| CoreAbility.Ability
		|| SpecialAbility.Ability
		|| EnhanceAbility.Ability
		|| UltimateAbility.Ability;
}

const FPlayerAbilitySlotConfig* UPlayerAbilitySetConfig::FindSlotConfig(EAbilitySlot Slot) const
{
	switch (Slot)
	{
	case EAbilitySlot::Passive:
		return &PassiveAbility;
	case EAbilitySlot::Primary:
		return &PrimaryAbility;
	case EAbilitySlot::PrimaryHeavy:
		return &PrimaryHeavyAbility;
	case EAbilitySlot::Secondary:
		return &SecondaryAbility;
	case EAbilitySlot::SecondaryHeavy:
		return &SecondaryHeavyAbility;
	case EAbilitySlot::Core:
		return &CoreAbility;
	case EAbilitySlot::Special:
		return &SpecialAbility;
	case EAbilitySlot::Signature:
		return &SignatureAbility;
	case EAbilitySlot::Enhance:
		return &EnhanceAbility;
	case EAbilitySlot::Ultimate:
		return &UltimateAbility;
	default:
		return nullptr;
	}
}

FPlayerAbilitySlotConfig* UPlayerAbilitySetConfig::FindMutableSlotConfig(EAbilitySlot Slot)
{
	switch (Slot)
	{
	case EAbilitySlot::Passive:
		return &PassiveAbility;
	case EAbilitySlot::Primary:
		return &PrimaryAbility;
	case EAbilitySlot::PrimaryHeavy:
		return &PrimaryHeavyAbility;
	case EAbilitySlot::Secondary:
		return &SecondaryAbility;
	case EAbilitySlot::SecondaryHeavy:
		return &SecondaryHeavyAbility;
	case EAbilitySlot::Core:
		return &CoreAbility;
	case EAbilitySlot::Special:
		return &SpecialAbility;
	case EAbilitySlot::Signature:
		return &SignatureAbility;
	case EAbilitySlot::Enhance:
		return &EnhanceAbility;
	case EAbilitySlot::Ultimate:
		return &UltimateAbility;
	default:
		return nullptr;
	}
}

UAbilityDefinitionConfig* UPlayerAbilitySetConfig::CreateGeneratedDefinition(FName BaseName)
{
	const FName UniqueName = MakeUniqueObjectName(this, UAbilityDefinitionConfig::StaticClass(), BaseName);
	UAbilityDefinitionConfig* AbilityDefinition =
		NewObject<UAbilityDefinitionConfig>(this, UAbilityDefinitionConfig::StaticClass(), UniqueName, RF_Transactional);

	if (AbilityDefinition)
	{
		GeneratedAbilityDefinitions.Add(AbilityDefinition);
	}

	return AbilityDefinition;
}

UAbilityDefinitionConfig* UPlayerAbilitySetConfig::FindOrCreateGeneratedDefinition(EAbilitySlot Slot, FName BaseName)
{
	if (FPlayerAbilitySlotConfig* SlotConfig = FindMutableSlotConfig(Slot))
	{
		if (SlotConfig->Ability)
		{
			return SlotConfig->Ability;
		}
	}

	UAbilityDefinitionConfig* AbilityDefinition = CreateGeneratedDefinition(BaseName);
	AssignAbilityToSlot(Slot, AbilityDefinition);
	return AbilityDefinition;
}

void UPlayerAbilitySetConfig::AssignAbilityToSlot(
	EAbilitySlot Slot,
	UAbilityDefinitionConfig* AbilityDefinition,
	bool bAutoGrant)
{
	FPlayerAbilitySlotConfig* SlotConfig = FindMutableSlotConfig(Slot);
	if (!SlotConfig) return;

	SlotConfig->Ability = AbilityDefinition;
	SlotConfig->bAutoGrant = bAutoGrant;
}

void UPlayerAbilitySetConfig::ApplyDeprecatedAbilityEntries()
{
	for (const FDeprecatedPlayerAbilityEntry& DeprecatedAbility : Abilities)
	{
		UAbilityDefinitionConfig* AbilityDefinition =
			CreateGeneratedDefinition(FName(*FString::Printf(TEXT("MigratedAbility_%d"), static_cast<int32>(DeprecatedAbility.Slot))));
		if (!AbilityDefinition) continue;

		AbilityDefinition->AbilityID = ResolveDeprecatedAbilityID(DeprecatedAbility);
		AbilityDefinition->AbilityClass = ResolveDeprecatedAbilityClass(DeprecatedAbility);
		AbilityDefinition->RequiredTags = DeprecatedAbility.RequiredTags;
		AbilityDefinition->BlockedTags = DeprecatedAbility.BlockedTags;
		AbilityDefinition->ActiveStateTag = DeprecatedAbility.ActiveStateTag;
		DuplicateFragments(AbilityDefinition, DeprecatedAbility.Fragments, AbilityDefinition->Fragments);

		if (DeprecatedAbility.AbilityType == EAbilityType::TwinSwordSignature
			|| DeprecatedAbility.Slot == EAbilitySlot::Signature)
		{
			FindOrAddFragment<UAbilityActionExecutionFragment>(*AbilityDefinition)->ExecutionMode =
				EAbilityActionExecutionMode::ForwardHit;
		}

		if (DeprecatedAbility.AbilityType == EAbilityType::TwinSwordEnhance
			|| DeprecatedAbility.Slot == EAbilitySlot::Enhance)
		{
			FindOrAddFragment<UAbilityActionExecutionFragment>(*AbilityDefinition)->ExecutionMode =
				EAbilityActionExecutionMode::ApplyTimedState;
		}

		AssignAbilityToSlot(
			DeprecatedAbility.Slot,
			AbilityDefinition,
			DeprecatedAbility.bAutoGrant
		);
	}
}

void UPlayerAbilitySetConfig::ApplyDeprecatedConfigData()
{
	if (const UTwinSwordCoreAbilityConfig* CoreConfig = Cast<UTwinSwordCoreAbilityConfig>(CoreAbilityConfig))
	{
		UAbilityDefinitionConfig* Core =
			FindOrCreateGeneratedDefinition(EAbilitySlot::Core, TEXT("MigratedAbility_TwinSword_PerfectDodge"));
		Core->AbilityClass = CoreConfig->AbilityClass;
		UAbilityMontageSectionsFragment* Sections = FindOrAddFragment<UAbilityMontageSectionsFragment>(*Core);
		Sections->Montage = CoreConfig->DodgeMontage;
		Sections->Sections.AddDefaulted();
	}

	auto ApplyComboConfig = [this](EAbilitySlot Slot, const UBaseAbilityConfig* DeprecatedConfig, FName BaseName)
	{
		const UTwinSwordComboAbilityConfig* ComboConfig = Cast<UTwinSwordComboAbilityConfig>(DeprecatedConfig);
		if (!ComboConfig) return;

		UAbilityDefinitionConfig* AbilityDefinition = FindOrCreateGeneratedDefinition(Slot, BaseName);
		if (!AbilityDefinition) return;

		AbilityDefinition->AbilityClass = ComboConfig->AbilityClass;

		if (ComboConfig->StaminaCost > 0.0f)
		{
			FindOrAddFragment<UAbilityResourceCostFragment>(*AbilityDefinition)->Stamina = ComboConfig->StaminaCost;
		}

		UAbilityHitDetectionFragment* Hit = FindOrAddFragment<UAbilityHitDetectionFragment>(*AbilityDefinition);
		Hit->Radius = ComboConfig->HitRadius;
		Hit->Range = ComboConfig->HitRange;
		Hit->bDrawDebug = ComboConfig->bDrawDebugHitCheck;

		UAbilityMontageSectionsFragment* Combo = FindOrAddFragment<UAbilityMontageSectionsFragment>(*AbilityDefinition);
		Combo->Montage = ComboConfig->AttackMontage;
		Combo->Sections.Reset();
		for (int32 Index = 0; Index < ComboConfig->ComboSections.Num(); ++Index)
		{
			FAbilityMontageSection& Section = Combo->Sections.AddDefaulted_GetRef();
			Section.SectionName = ComboConfig->ComboSections[Index];
			FAbilityEffectConfig& DamageEffect = Section.Effects.AddDefaulted_GetRef();
			DamageEffect.Type = EAbilityEffectKind::Damage;
			DamageEffect.Trigger = EAbilityEffectTriggerEvent::OnHit;
			DamageEffect.Magnitude = ComboConfig->ComboDamages.IsValidIndex(Index)
				? ComboConfig->ComboDamages[Index]
				: 0.0f;
		}
	};

	ApplyComboConfig(EAbilitySlot::Primary, PrimaryAbilityConfig, TEXT("MigratedAbility_TwinSword_Combo1"));
	ApplyComboConfig(EAbilitySlot::Secondary, SecondaryAbilityConfig, TEXT("MigratedAbility_TwinSword_Combo2"));

	CoreAbilityConfig = nullptr;
	PrimaryAbilityConfig = nullptr;
	SecondaryAbilityConfig = nullptr;
	SignatureAbilityConfig = nullptr;
	EnhanceAbilityConfig = nullptr;
	UltimateAbilityConfig = nullptr;
	PassiveAbilities.Empty();
}
