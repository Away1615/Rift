#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Attributes/RiftAttributeMacros.h"
#include "RiftResourceAttributeSet.generated.h"

UCLASS()
class RIFT_API URiftResourceAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	URiftResourceAttributeSet();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_SwordIntent, Category="Attributes|SwordIntent")
	FGameplayAttributeData SwordIntent;
	ATTRIBUTE_ACCESSORS(URiftResourceAttributeSet, SwordIntent)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_MaxSwordIntent, Category="Attributes|SwordIntent")
	FGameplayAttributeData MaxSwordIntent;
	ATTRIBUTE_ACCESSORS(URiftResourceAttributeSet, MaxSwordIntent)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_UltimateCharge, Category="Attributes|Ultimate")
	FGameplayAttributeData UltimateCharge;
	ATTRIBUTE_ACCESSORS(URiftResourceAttributeSet, UltimateCharge)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_MaxUltimateCharge, Category="Attributes|Ultimate")
	FGameplayAttributeData MaxUltimateCharge;
	ATTRIBUTE_ACCESSORS(URiftResourceAttributeSet, MaxUltimateCharge)

protected:
	UFUNCTION()
	void OnRep_SwordIntent(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_MaxSwordIntent(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_UltimateCharge(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_MaxUltimateCharge(const FGameplayAttributeData& OldValue);
};
