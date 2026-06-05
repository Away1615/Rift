// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Data/Player/Input/AbilityInputID.h"
#include "Engine/DataAsset.h"
#include "PlayerAbilitySetConfig.generated.h"

class UBaseGameplayAbility;
class UBaseAbilityConfig;
class UAnimMontage;
class UGameplayEffect;
class UParticleSystem;
class USoundBase;

UENUM(BlueprintType)
enum class ERiftAbilitySlot : uint8
{
	Passive		UMETA(DisplayName="Passive"),
	Primary		UMETA(DisplayName="Primary"),
	Secondary	UMETA(DisplayName="Secondary"),
	Core		UMETA(DisplayName="Core"),
	Signature	UMETA(DisplayName="Signature"),
	Enhance		UMETA(DisplayName="Enhance"),
	Ultimate	UMETA(DisplayName="Ultimate")
};

UENUM(BlueprintType)
enum class ERiftAbilityType : uint8
{
	None				UMETA(DisplayName="None"),
	TwinSwordCombo		UMETA(DisplayName="Twin Sword Combo"),
	TwinSwordDodge		UMETA(DisplayName="Twin Sword Dodge"),
	TwinSwordSignature	UMETA(DisplayName="Twin Sword Signature"),
	TwinSwordEnhance	UMETA(DisplayName="Twin Sword Enhance")
};

USTRUCT(BlueprintType)
struct FRiftComboStepSpec
{
	GENERATED_BODY()

	FRiftComboStepSpec() = default;

	FRiftComboStepSpec(FName InSectionName, float InDamage)
		: SectionName(InSectionName)
		, Damage(InDamage)
	{
	}

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combo")
	FName SectionName = NAME_None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combo", meta=(ClampMin="0.0"))
	float Damage = 0.0f;
};

UCLASS(Abstract, BlueprintType, EditInlineNew, DefaultToInstanced)
class RIFTWARD_API URiftAbilityFragment : public UObject
{
	GENERATED_BODY()
};

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class RIFTWARD_API URiftAbilityCostFragment : public URiftAbilityFragment
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Cost", meta=(ClampMin="0.0"))
	float Stamina = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Cost", meta=(ClampMin="0.0"))
	float Mana = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Cost")
	TSubclassOf<UGameplayEffect> CostEffectClass;
};

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class RIFTWARD_API URiftAbilityMontageFragment : public URiftAbilityFragment
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Animation")
	TObjectPtr<UAnimMontage> Montage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Animation")
	TObjectPtr<UAnimMontage> EmpoweredMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Animation")
	FName StartSection = NAME_None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Animation", meta=(ClampMin="0.01"))
	float PlayRate = 1.0f;
};

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class RIFTWARD_API URiftAbilityHitFragment : public URiftAbilityFragment
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Hit", meta=(ClampMin="0.0"))
	float Damage = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Hit", meta=(ClampMin="0.0"))
	float Range = 450.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Hit", meta=(ClampMin="1.0"))
	float Radius = 80.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Hit", meta=(ClampMin="0.01"))
	float EnhancedRadiusMultiplier = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Hit")
	bool bDrawDebug = false;
};

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class RIFTWARD_API URiftAbilityComboFragment : public URiftAbilityFragment
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combo")
	TArray<FRiftComboStepSpec> Steps;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combo", meta=(ClampMin="0.0"))
	float EmpoweredDamageMultiplier = 1.0f;
};

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class RIFTWARD_API URiftAbilityBuffFragment : public URiftAbilityFragment
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Buff")
	FGameplayTag ActiveStateTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Buff", meta=(ClampMin="0.0"))
	float Duration = 0.0f;
};

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class RIFTWARD_API URiftAbilityDodgeFragment : public URiftAbilityFragment
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Perfect Dodge", meta=(ClampMin="0.0"))
	float PerfectWindowDuration = 0.25f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Perfect Dodge")
	TObjectPtr<UAnimMontage> PerfectSuccessMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Perfect Dodge")
	FName PerfectSuccessSection = NAME_None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Perfect Dodge", meta=(ClampMin="0.01"))
	float PerfectSuccessPlayRate = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Perfect Dodge|Cue")
	TObjectPtr<UParticleSystem> PerfectSuccessEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Perfect Dodge|Cue")
	TObjectPtr<USoundBase> PerfectSuccessSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Perfect Dodge|Cue")
	FName CueSocketName = NAME_None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Perfect Dodge|Cue")
	FVector CueLocationOffset = FVector::ZeroVector;
};

USTRUCT(BlueprintType)
struct FPlayerAbilityEntry
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability")
	FGameplayTag AbilityID;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability")
	ERiftAbilitySlot Slot = ERiftAbilitySlot::Passive;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability")
	ERiftAbilityType AbilityType = ERiftAbilityType::None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability")
	EAbilityInputID InputID = EAbilityInputID::None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability")
	TSubclassOf<UBaseGameplayAbility> AbilityClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability", meta=(ClampMin="1"))
	int32 Level = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability")
	bool bAutoGrant = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Tags")
	FGameplayTagContainer RequiredTags;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Tags")
	FGameplayTagContainer BlockedTags;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Tags")
	FGameplayTag ActiveStateTag;

	UPROPERTY(EditDefaultsOnly, Instanced, BlueprintReadOnly, Category="Fragments")
	TArray<TObjectPtr<URiftAbilityFragment>> Fragments;

	template <typename FragmentType>
	const FragmentType* FindFragment() const
	{
		for (const TObjectPtr<URiftAbilityFragment>& Fragment : Fragments)
		{
			if (const FragmentType* TypedFragment = Cast<FragmentType>(Fragment))
			{
				return TypedFragment;
			}
		}
		return nullptr;
	}

	template <typename FragmentType>
	FragmentType* FindMutableFragment()
	{
		for (const TObjectPtr<URiftAbilityFragment>& Fragment : Fragments)
		{
			if (FragmentType* TypedFragment = Cast<FragmentType>(Fragment))
			{
				return TypedFragment;
			}
		}
		return nullptr;
	}
};

/**
 *
 */
UCLASS(BlueprintType)
class RIFTWARD_API UPlayerAbilitySetConfig : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPlayerAbilitySetConfig();
	virtual void PostLoad() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability")
	TArray<FPlayerAbilityEntry> Abilities;

	const FPlayerAbilityEntry* FindAbilityByID(FGameplayTag AbilityID) const;
	const FPlayerAbilityEntry* FindAbilityByInputID(EAbilityInputID InputID) const;
	TSubclassOf<UBaseGameplayAbility> ResolveAbilityClass(const FPlayerAbilityEntry& AbilityEntry) const;

private:
	UPROPERTY()
	TArray<TSubclassOf<UBaseGameplayAbility>> PassiveAbilities;

	UPROPERTY()
	TObjectPtr<UBaseAbilityConfig> CoreAbilityConfig;

	UPROPERTY()
	TObjectPtr<UBaseAbilityConfig> PrimaryAbilityConfig;

	UPROPERTY()
	TObjectPtr<UBaseAbilityConfig> SecondaryAbilityConfig;

	UPROPERTY()
	TObjectPtr<UBaseAbilityConfig> SignatureAbilityConfig;

	UPROPERTY()
	TObjectPtr<UBaseAbilityConfig> EnhanceAbilityConfig;

	UPROPERTY()
	TObjectPtr<UBaseAbilityConfig> UltimateAbilityConfig;

	FPlayerAbilityEntry* FindMutableAbilityBySlot(ERiftAbilitySlot Slot);
	template <typename FragmentType>
	FragmentType* FindOrAddFragment(FPlayerAbilityEntry& AbilityEntry);
	void EnsureDefaultTwinSwordEntries();
	void ApplyDeprecatedConfigData();

};

template <typename FragmentType>
FragmentType* UPlayerAbilitySetConfig::FindOrAddFragment(FPlayerAbilityEntry& AbilityEntry)
{
	if (FragmentType* Fragment = AbilityEntry.FindMutableFragment<FragmentType>())
	{
		return Fragment;
	}

	FragmentType* Fragment = NewObject<FragmentType>(this, FragmentType::StaticClass(), NAME_None, RF_Transactional);
	AbilityEntry.Fragments.Add(Fragment);
	return Fragment;
}
