#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "RiftAudioConfig.generated.h"

class USoundBase;
class USoundClass;
class USoundMix;
class UWorld;

UENUM(BlueprintType)
enum class ERiftUISoundType : uint8
{
	ButtonHover UMETA(DisplayName="Button Hover"),
	ButtonClick UMETA(DisplayName="Button Click"),
	Back UMETA(DisplayName="Back"),
	Error UMETA(DisplayName="Error")
};

UCLASS(BlueprintType)
class RIFT_API URiftAudioConfig : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Frontend")
	TObjectPtr<USoundBase> FrontendMusic;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Frontend")
	TArray<TSoftObjectPtr<UWorld>> FrontendMaps;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Frontend", meta=(ClampMin="0.0"))
	float FrontendMusicVolume = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Frontend", meta=(ClampMin="0.0"))
	float FrontendMusicPitch = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Frontend", meta=(ClampMin="0.0"))
	float FrontendMusicFadeOutDuration = 0.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Frontend")
	bool bStopFrontendMusicOutsideFrontendMaps = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UI")
	TObjectPtr<USoundBase> ButtonHoverSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UI")
	TObjectPtr<USoundBase> ButtonClickSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UI")
	TObjectPtr<USoundBase> BackSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UI")
	TObjectPtr<USoundBase> ErrorSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UI", meta=(ClampMin="0.0"))
	float UISoundVolume = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UI", meta=(ClampMin="0.0"))
	float UISoundPitch = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Mix")
	TObjectPtr<USoundMix> DefaultSoundMix;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Mix")
	TObjectPtr<USoundClass> MasterSoundClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Mix")
	TObjectPtr<USoundClass> MusicSoundClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Mix")
	TObjectPtr<USoundClass> SFXSoundClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Mix")
	TObjectPtr<USoundClass> UISoundClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Mix")
	TObjectPtr<USoundClass> AmbienceSoundClass;

	bool IsFrontendMap(const FString& WorldPackageName) const;
	USoundBase* GetUISound(ERiftUISoundType SoundType) const;
};
