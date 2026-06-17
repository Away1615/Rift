// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Static.h"
#include "RiftGameplayCueNotify_CombatImpact.generated.h"

class UNiagaraSystem;
class USoundBase;

UCLASS(Blueprintable)
class RIFT_API URiftGameplayCueNotify_CombatImpact : public UGameplayCueNotify_Static
{
	GENERATED_BODY()

public:
	virtual bool OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Rift|Combat Impact")
	TObjectPtr<UNiagaraSystem> ImpactNiagara;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Rift|Combat Impact")
	TObjectPtr<USoundBase> ImpactSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Rift|Combat Impact")
	float NiagaraScale = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Rift|Combat Impact")
	float VolumeMultiplier = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Rift|Combat Impact")
	float PitchMultiplier = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Rift|Combat Impact")
	bool bUseImpactNormalRotation = true;
};
