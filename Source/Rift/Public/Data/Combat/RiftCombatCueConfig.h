// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "RiftCombatCueConfig.generated.h"

class UNiagaraSystem;
class USoundBase;

UCLASS(BlueprintType)
class RIFT_API URiftCombatCueConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Hit")
	TObjectPtr<UNiagaraSystem> HitNiagara;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Hit")
	TObjectPtr<USoundBase> HitSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Blocked")
	TObjectPtr<UNiagaraSystem> BlockedNiagara;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Blocked")
	TObjectPtr<USoundBase> BlockedSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Playback")
	float NiagaraScale = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Playback")
	float VolumeMultiplier = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Playback")
	float PitchMultiplier = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Playback")
	bool bUseImpactNormalRotation = true;

	void PlayHit(UWorld* World, const FVector& Location, const FVector& Normal) const;
	void PlayBlocked(UWorld* World, const FVector& Location, const FVector& Normal) const;

private:
	void PlayImpact(
		UWorld* World,
		UNiagaraSystem* Niagara,
		USoundBase* Sound,
		const FVector& Location,
		const FVector& Normal
	) const;
};
