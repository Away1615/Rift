// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Data/Player/Ability/AbilityDefinitionConfig.h"
#include "Data/Player/Input/AbilityInputID.h"
#include "Engine/DataAsset.h"
#include "PlayerAbilitySetConfig.generated.h"

class UBaseAbilityConfig;
class UBaseGameplayAbility;

UENUM(BlueprintType)
enum class EAbilitySlot : uint8
{
	Passive			UMETA(DisplayName="Passive"),
	Primary			UMETA(DisplayName="Primary"),
	PrimaryHeavy	UMETA(DisplayName="Primary Heavy"),
	Secondary		UMETA(DisplayName="Secondary"),
	SecondaryHeavy	UMETA(DisplayName="Secondary Heavy"),
	Core			UMETA(DisplayName="Core"),
	Special			UMETA(DisplayName="Special"),
	Enhance			UMETA(DisplayName="Enhance"),
	Ultimate		UMETA(DisplayName="Ultimate"),
	Signature		UMETA(Hidden, DisplayName="Deprecated Signature")
};

UENUM(BlueprintType)
enum class EAbilityType : uint8
{
	None				UMETA(DisplayName="None"),
	TwinSwordCombo		UMETA(DisplayName="Twin Sword Combo"),
	TwinSwordDodge		UMETA(DisplayName="Twin Sword Dodge"),
	TwinSwordSignature	UMETA(DisplayName="Twin Sword Signature"),
	TwinSwordEnhance	UMETA(DisplayName="Twin Sword Enhance")
};

USTRUCT(BlueprintType)
struct FPlayerAbilitySlotConfig
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability")
	TObjectPtr<UAbilityDefinitionConfig> Ability;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability", meta=(AdvancedDisplay))
	bool bAutoGrant = true;
};

USTRUCT(BlueprintType)
struct FPlayerAbilityGrant
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category="Ability")
	EAbilitySlot Slot = EAbilitySlot::Passive;

	UPROPERTY(BlueprintReadOnly, Category="Ability")
	TObjectPtr<UAbilityDefinitionConfig> Ability;

	UPROPERTY(BlueprintReadOnly, Category="Ability")
	bool bAutoGrant = true;
};

USTRUCT()
struct FDeprecatedPlayerAbilityEntry
{
	GENERATED_BODY()

	UPROPERTY()
	FGameplayTag AbilityID;

	UPROPERTY()
	EAbilitySlot Slot = EAbilitySlot::Passive;

	UPROPERTY()
	EAbilityType AbilityType = EAbilityType::None;

	UPROPERTY()
	EAbilityInputID InputID = EAbilityInputID::None;

	UPROPERTY()
	TSubclassOf<UBaseGameplayAbility> AbilityClass;

	UPROPERTY()
	bool bAutoGrant = true;

	UPROPERTY()
	FGameplayTagContainer RequiredTags;

	UPROPERTY()
	FGameplayTagContainer BlockedTags;

	UPROPERTY()
	FGameplayTag ActiveStateTag;

	UPROPERTY(Instanced)
	TArray<TObjectPtr<UAbilityConfigFragment>> Fragments;
};

/**
 * Per-class ability loadout. This asset owns only slot bindings; concrete skill
 * identity, class, tags, and fragments live on UAbilityDefinitionConfig.
 */
UCLASS(BlueprintType)
class RIFTWARD_API UPlayerAbilitySetConfig : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPlayerAbilitySetConfig();
	virtual void PostLoad() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability|Slots")
	FPlayerAbilitySlotConfig PassiveAbility;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability|Slots")
	FPlayerAbilitySlotConfig PrimaryAbility;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability|Slots")
	FPlayerAbilitySlotConfig PrimaryHeavyAbility;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability|Slots")
	FPlayerAbilitySlotConfig SecondaryAbility;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability|Slots")
	FPlayerAbilitySlotConfig SecondaryHeavyAbility;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability|Slots")
	FPlayerAbilitySlotConfig CoreAbility;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability|Slots")
	FPlayerAbilitySlotConfig SpecialAbility;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability|Slots")
	FPlayerAbilitySlotConfig EnhanceAbility;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability|Slots")
	FPlayerAbilitySlotConfig UltimateAbility;

	void GetGrantableAbilities(TArray<FPlayerAbilityGrant>& OutAbilities) const;

	const UAbilityDefinitionConfig* FindAbilityByID(FGameplayTag AbilityID) const;
	const UAbilityDefinitionConfig* FindAbilityByInputID(EAbilityInputID InputID) const;
	const UAbilityDefinitionConfig* FindAbilityBySlot(EAbilitySlot Slot) const;

	static EAbilityInputID GetInputIDForSlot(EAbilitySlot Slot);
	static EAbilitySlot GetSlotForInputID(EAbilityInputID InputID);

private:
	UPROPERTY(meta=(DeprecatedProperty))
	FPlayerAbilitySlotConfig SignatureAbility;

	UPROPERTY(meta=(DeprecatedProperty))
	TArray<FDeprecatedPlayerAbilityEntry> Abilities;

	UPROPERTY(meta=(DeprecatedProperty))
	TArray<TSubclassOf<UBaseGameplayAbility>> PassiveAbilities;

	UPROPERTY(meta=(DeprecatedProperty))
	TObjectPtr<UBaseAbilityConfig> CoreAbilityConfig;

	UPROPERTY(meta=(DeprecatedProperty))
	TObjectPtr<UBaseAbilityConfig> PrimaryAbilityConfig;

	UPROPERTY(meta=(DeprecatedProperty))
	TObjectPtr<UBaseAbilityConfig> SecondaryAbilityConfig;

	UPROPERTY(meta=(DeprecatedProperty))
	TObjectPtr<UBaseAbilityConfig> SignatureAbilityConfig;

	UPROPERTY(meta=(DeprecatedProperty))
	TObjectPtr<UBaseAbilityConfig> EnhanceAbilityConfig;

	UPROPERTY(meta=(DeprecatedProperty))
	TObjectPtr<UBaseAbilityConfig> UltimateAbilityConfig;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UAbilityDefinitionConfig>> GeneratedAbilityDefinitions;

	bool HasAnyConfiguredAbility() const;
	const FPlayerAbilitySlotConfig* FindSlotConfig(EAbilitySlot Slot) const;
	FPlayerAbilitySlotConfig* FindMutableSlotConfig(EAbilitySlot Slot);

	UAbilityDefinitionConfig* CreateGeneratedDefinition(FName BaseName);
	UAbilityDefinitionConfig* FindOrCreateGeneratedDefinition(EAbilitySlot Slot, FName BaseName);
	void AssignAbilityToSlot(
		EAbilitySlot Slot,
		UAbilityDefinitionConfig* AbilityDefinition,
		bool bAutoGrant = true
	);

	template <typename FragmentType>
	FragmentType* FindOrAddFragment(UAbilityDefinitionConfig& AbilityDefinition);

	void ApplyDeprecatedAbilityEntries();
	void ApplyDeprecatedConfigData();
};

template <typename FragmentType>
FragmentType* UPlayerAbilitySetConfig::FindOrAddFragment(UAbilityDefinitionConfig& AbilityDefinition)
{
	if (FragmentType* Fragment = AbilityDefinition.FindMutableFragment<FragmentType>())
	{
		return Fragment;
	}

	FragmentType* Fragment = NewObject<FragmentType>(&AbilityDefinition, FragmentType::StaticClass(), NAME_None, RF_Transactional);
	AbilityDefinition.Fragments.Add(Fragment);
	return Fragment;
}
