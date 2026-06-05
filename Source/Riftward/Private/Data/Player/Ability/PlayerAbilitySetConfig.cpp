// Fill out your copyright notice in the Description page of Project Settings.


#include "Data/Player/Ability/PlayerAbilitySetConfig.h"

#include "AbilitySystem/GameplayAbilities/GA_TwinSword_Action.h"
#include "AbilitySystem/GameplayAbilities/GA_TwinSword_Core.h"
#include "AbilitySystem/GameplayAbilities/GA_TwinSword_Primary.h"
#include "AbilitySystem/GameplayAbilities/GA_TwinSword_Secondary.h"
#include "Animation/AnimMontage.h"
#include "Data/Player/Ability/BaseAbilityConfig.h"
#include "Data/Player/Ability/TwinSword/TwinSwordComboAbilityConfig.h"
#include "Data/Player/Ability/TwinSword/TwinSwordCoreAbilityConfig.h"
#include "GameplayTags/RiftGameplayTags.h"

UPlayerAbilitySetConfig::UPlayerAbilitySetConfig()
{
}

void UPlayerAbilitySetConfig::PostLoad()
{
	Super::PostLoad();
	EnsureDefaultTwinSwordEntries();
	ApplyDeprecatedConfigData();
}

const FPlayerAbilityEntry* UPlayerAbilitySetConfig::FindAbilityByID(FGameplayTag AbilityID) const
{
	if (!AbilityID.IsValid()) return nullptr;

	return Abilities.FindByPredicate([AbilityID](const FPlayerAbilityEntry& AbilityEntry)
	{
		return AbilityEntry.AbilityID == AbilityID;
	});
}

const FPlayerAbilityEntry* UPlayerAbilitySetConfig::FindAbilityByInputID(EAbilityInputID InputID) const
{
	if (InputID == EAbilityInputID::None) return nullptr;

	return Abilities.FindByPredicate([InputID](const FPlayerAbilityEntry& AbilityEntry)
	{
		return AbilityEntry.InputID == InputID;
	});
}

TSubclassOf<UBaseGameplayAbility> UPlayerAbilitySetConfig::ResolveAbilityClass(
	const FPlayerAbilityEntry& AbilityEntry) const
{
	if (AbilityEntry.AbilityClass)
	{
		return AbilityEntry.AbilityClass;
	}

	switch (AbilityEntry.AbilityType)
	{
	case ERiftAbilityType::TwinSwordCombo:
		return AbilityEntry.Slot == ERiftAbilitySlot::Secondary
			? UGA_TwinSword_Secondary::StaticClass()
			: UGA_TwinSword_Primary::StaticClass();
	case ERiftAbilityType::TwinSwordDodge:
		return UGA_TwinSword_Core::StaticClass();
	case ERiftAbilityType::TwinSwordSignature:
	case ERiftAbilityType::TwinSwordEnhance:
		return UGA_TwinSword_Action::StaticClass();
	default:
		return nullptr;
	}
}

FPlayerAbilityEntry* UPlayerAbilitySetConfig::FindMutableAbilityBySlot(ERiftAbilitySlot Slot)
{
	return Abilities.FindByPredicate([Slot](const FPlayerAbilityEntry& AbilityEntry)
	{
		return AbilityEntry.Slot == Slot;
	});
}

void UPlayerAbilitySetConfig::EnsureDefaultTwinSwordEntries()
{
	if (!Abilities.IsEmpty()) return;

	const FRiftGameplayTags& Tags = FRiftGameplayTags::Get();

	FPlayerAbilityEntry& Core = Abilities.AddDefaulted_GetRef();
	Core.AbilityID = Tags.Ability_TwinSword_Core;
	Core.Slot = ERiftAbilitySlot::Core;
	Core.AbilityType = ERiftAbilityType::TwinSwordDodge;
	Core.InputID = EAbilityInputID::Core;
	Core.AbilityClass = UGA_TwinSword_Core::StaticClass();
	Core.ActiveStateTag = Tags.State_Ability_TwinSword_Core_DodgeActive;
	Core.BlockedTags.AddTag(Tags.State_Ability_TwinSword_Core_DodgeActive);
	URiftAbilityMontageFragment* CoreMontage = FindOrAddFragment<URiftAbilityMontageFragment>(Core);
	CoreMontage->Montage = LoadObject<UAnimMontage>(
		nullptr,
		TEXT("/Game/0_/Classes/TwinSword/Animation/Montages/AM_TwinSword_Dodge.AM_TwinSword_Dodge")
	);
	CoreMontage->StartSection = FName(TEXT("Dodge"));
	URiftAbilityDodgeFragment* Dodge = FindOrAddFragment<URiftAbilityDodgeFragment>(Core);
	Dodge->PerfectWindowDuration = 0.25f;

	FPlayerAbilityEntry& Primary = Abilities.AddDefaulted_GetRef();
	Primary.AbilityID = Tags.Ability_TwinSword_Primary;
	Primary.Slot = ERiftAbilitySlot::Primary;
	Primary.AbilityType = ERiftAbilityType::TwinSwordCombo;
	Primary.InputID = EAbilityInputID::Primary;
	Primary.AbilityClass = UGA_TwinSword_Primary::StaticClass();
	Primary.ActiveStateTag = Tags.State_Ability_TwinSword_Primary_ComboActive;
	Primary.BlockedTags.AddTag(Tags.State_Ability_TwinSword_Core_DodgeActive);
	Primary.BlockedTags.AddTag(Tags.State_Ability_TwinSword_Primary_ComboActive);
	Primary.BlockedTags.AddTag(Tags.State_Ability_TwinSword_Secondary_ComboActive);
	URiftAbilityMontageFragment* PrimaryMontage = FindOrAddFragment<URiftAbilityMontageFragment>(Primary);
	PrimaryMontage->Montage = LoadObject<UAnimMontage>(
		nullptr,
		TEXT("/Game/0_/Classes/TwinSword/Animation/Montages/AM_TwinSword_Combo01.AM_TwinSword_Combo01")
	);
	URiftAbilityHitFragment* PrimaryHit = FindOrAddFragment<URiftAbilityHitFragment>(Primary);
	PrimaryHit->Radius = 80.0f;
	PrimaryHit->EnhancedRadiusMultiplier = 1.5f;
	URiftAbilityComboFragment* PrimaryCombo = FindOrAddFragment<URiftAbilityComboFragment>(Primary);
	PrimaryCombo->EmpoweredDamageMultiplier = 1.35f;
	PrimaryCombo->Steps = {
		{FName(TEXT("Attack1")), 15.0f},
		{FName(TEXT("Attack2")), 20.0f},
		{FName(TEXT("Attack3")), 28.0f}
	};

	FPlayerAbilityEntry& Secondary = Abilities.AddDefaulted_GetRef();
	Secondary.AbilityID = Tags.Ability_TwinSword_Secondary;
	Secondary.Slot = ERiftAbilitySlot::Secondary;
	Secondary.AbilityType = ERiftAbilityType::TwinSwordCombo;
	Secondary.InputID = EAbilityInputID::Secondary;
	Secondary.AbilityClass = UGA_TwinSword_Secondary::StaticClass();
	Secondary.ActiveStateTag = Tags.State_Ability_TwinSword_Secondary_ComboActive;
	Secondary.BlockedTags.AddTag(Tags.State_Ability_TwinSword_Core_DodgeActive);
	Secondary.BlockedTags.AddTag(Tags.State_Ability_TwinSword_Primary_ComboActive);
	Secondary.BlockedTags.AddTag(Tags.State_Ability_TwinSword_Secondary_ComboActive);
	URiftAbilityCostFragment* SecondaryCost = FindOrAddFragment<URiftAbilityCostFragment>(Secondary);
	SecondaryCost->Stamina = 20.0f;
	URiftAbilityMontageFragment* SecondaryMontage = FindOrAddFragment<URiftAbilityMontageFragment>(Secondary);
	SecondaryMontage->Montage = LoadObject<UAnimMontage>(
		nullptr,
		TEXT("/Game/0_/Classes/TwinSword/Animation/Montages/AM_TwinSword_Combo02.AM_TwinSword_Combo02")
	);
	URiftAbilityHitFragment* SecondaryHit = FindOrAddFragment<URiftAbilityHitFragment>(Secondary);
	SecondaryHit->Radius = 90.0f;
	SecondaryHit->EnhancedRadiusMultiplier = 1.5f;
	URiftAbilityComboFragment* SecondaryCombo = FindOrAddFragment<URiftAbilityComboFragment>(Secondary);
	SecondaryCombo->EmpoweredDamageMultiplier = 1.35f;
	SecondaryCombo->Steps = {
		{FName(TEXT("Attack1")), 24.0f},
		{FName(TEXT("Attack2")), 30.0f},
		{FName(TEXT("Attack3")), 36.0f},
		{FName(TEXT("Attack4")), 50.0f}
	};

	FPlayerAbilityEntry& Signature = Abilities.AddDefaulted_GetRef();
	Signature.AbilityID = Tags.Ability_TwinSword_Signature;
	Signature.Slot = ERiftAbilitySlot::Signature;
	Signature.AbilityType = ERiftAbilityType::TwinSwordSignature;
	Signature.InputID = EAbilityInputID::Signature;
	Signature.AbilityClass = UGA_TwinSword_Action::StaticClass();
	Signature.BlockedTags.AddTag(Tags.State_Ability_TwinSword_Core_DodgeActive);
	Signature.BlockedTags.AddTag(Tags.State_Ability_TwinSword_Primary_ComboActive);
	Signature.BlockedTags.AddTag(Tags.State_Ability_TwinSword_Secondary_ComboActive);
	URiftAbilityCostFragment* SignatureCost = FindOrAddFragment<URiftAbilityCostFragment>(Signature);
	SignatureCost->Stamina = 15.0f;
	SignatureCost->Mana = 10.0f;
	URiftAbilityHitFragment* SignatureHit = FindOrAddFragment<URiftAbilityHitFragment>(Signature);
	SignatureHit->Damage = 35.0f;
	SignatureHit->Range = 500.0f;
	SignatureHit->Radius = 120.0f;

	FPlayerAbilityEntry& Enhance = Abilities.AddDefaulted_GetRef();
	Enhance.AbilityID = Tags.Ability_TwinSword_Enhance;
	Enhance.Slot = ERiftAbilitySlot::Enhance;
	Enhance.AbilityType = ERiftAbilityType::TwinSwordEnhance;
	Enhance.InputID = EAbilityInputID::Enhance;
	Enhance.AbilityClass = UGA_TwinSword_Action::StaticClass();
	Enhance.BlockedTags.AddTag(Tags.State_Ability_TwinSword_Core_DodgeActive);
	Enhance.BlockedTags.AddTag(Tags.State_Ability_TwinSword_Primary_ComboActive);
	Enhance.BlockedTags.AddTag(Tags.State_Ability_TwinSword_Secondary_ComboActive);
	Enhance.BlockedTags.AddTag(Tags.State_Ability_TwinSword_Enhance_Active);
	URiftAbilityCostFragment* EnhanceCost = FindOrAddFragment<URiftAbilityCostFragment>(Enhance);
	EnhanceCost->Stamina = 10.0f;
	EnhanceCost->Mana = 20.0f;
	URiftAbilityBuffFragment* EnhanceBuff = FindOrAddFragment<URiftAbilityBuffFragment>(Enhance);
	EnhanceBuff->ActiveStateTag = Tags.State_Ability_TwinSword_Enhance_Active;
	EnhanceBuff->Duration = 8.0f;
}

void UPlayerAbilitySetConfig::ApplyDeprecatedConfigData()
{
	if (FPlayerAbilityEntry* Core = FindMutableAbilityBySlot(ERiftAbilitySlot::Core))
	{
		if (const UTwinSwordCoreAbilityConfig* CoreConfig = Cast<UTwinSwordCoreAbilityConfig>(CoreAbilityConfig))
		{
			Core->AbilityClass = CoreConfig->AbilityClass;
			FindOrAddFragment<URiftAbilityMontageFragment>(*Core)->Montage = CoreConfig->DodgeMontage;
		}
	}

	auto ApplyComboConfig = [this](ERiftAbilitySlot Slot, const UBaseAbilityConfig* DeprecatedConfig)
	{
		FPlayerAbilityEntry* AbilityEntry = FindMutableAbilityBySlot(Slot);
		const UTwinSwordComboAbilityConfig* ComboConfig = Cast<UTwinSwordComboAbilityConfig>(DeprecatedConfig);
		if (!AbilityEntry || !ComboConfig) return;

		AbilityEntry->AbilityClass = ComboConfig->AbilityClass;
		URiftAbilityMontageFragment* Montage = FindOrAddFragment<URiftAbilityMontageFragment>(*AbilityEntry);
		Montage->Montage = ComboConfig->AttackMontage;
		Montage->EmpoweredMontage = ComboConfig->SuperAttackMontage;

		if (ComboConfig->StaminaCost > 0.0f)
		{
			FindOrAddFragment<URiftAbilityCostFragment>(*AbilityEntry)->Stamina = ComboConfig->StaminaCost;
		}

		URiftAbilityHitFragment* Hit = FindOrAddFragment<URiftAbilityHitFragment>(*AbilityEntry);
		Hit->Radius = ComboConfig->HitRadius;
		Hit->Range = ComboConfig->HitRange;
		Hit->bDrawDebug = ComboConfig->bDrawDebugHitCheck;

		URiftAbilityComboFragment* Combo = FindOrAddFragment<URiftAbilityComboFragment>(*AbilityEntry);
		Combo->Steps.Reset();
		for (int32 Index = 0; Index < ComboConfig->ComboSections.Num(); ++Index)
		{
			FRiftComboStepSpec& Step = Combo->Steps.AddDefaulted_GetRef();
			Step.SectionName = ComboConfig->ComboSections[Index];
			Step.Damage = ComboConfig->ComboDamages.IsValidIndex(Index)
				? ComboConfig->ComboDamages[Index]
				: 0.0f;
		}
	};

	ApplyComboConfig(ERiftAbilitySlot::Primary, PrimaryAbilityConfig);
	ApplyComboConfig(ERiftAbilitySlot::Secondary, SecondaryAbilityConfig);

	CoreAbilityConfig = nullptr;
	PrimaryAbilityConfig = nullptr;
	SecondaryAbilityConfig = nullptr;
	SignatureAbilityConfig = nullptr;
	EnhanceAbilityConfig = nullptr;
	UltimateAbilityConfig = nullptr;
	PassiveAbilities.Empty();
}
