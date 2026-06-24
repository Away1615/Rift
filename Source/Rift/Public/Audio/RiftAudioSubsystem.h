#pragma once

#include "CoreMinimal.h"
#include "Data/Audio/RiftAudioConfig.h"
#include "Engine/World.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "RiftAudioSubsystem.generated.h"

class UAudioComponent;
class USoundClass;
class UWorld;

UCLASS()
class RIFT_API URiftAudioSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category="Rift|Audio")
	void RefreshMusicForWorld(UWorld* World);

	UFUNCTION(BlueprintCallable, Category="Rift|Audio")
	void PlayFrontendMusic();

	UFUNCTION(BlueprintCallable, Category="Rift|Audio")
	void StopFrontendMusic(float FadeOutDuration = -1.0f);

	UFUNCTION(BlueprintPure, Category="Rift|Audio")
	bool IsFrontendMusicPlaying() const;

	UFUNCTION(BlueprintCallable, Category="Rift|Audio|Mix")
	void ApplyDefaultSoundMix();

	UFUNCTION(BlueprintCallable, Category="Rift|Audio|Mix")
	void SetMasterVolume(float Volume);

	UFUNCTION(BlueprintCallable, Category="Rift|Audio|Mix")
	void SetMusicVolume(float Volume);

	UFUNCTION(BlueprintCallable, Category="Rift|Audio|Mix")
	void SetSFXVolume(float Volume);

	UFUNCTION(BlueprintCallable, Category="Rift|Audio|Mix")
	void SetUIVolume(float Volume);

	UFUNCTION(BlueprintCallable, Category="Rift|Audio|Mix")
	void SetAmbienceVolume(float Volume);

	UFUNCTION(BlueprintCallable, Category="Rift|Audio|UI")
	void PlayUISound(ERiftUISoundType SoundType);

	UFUNCTION(BlueprintCallable, Category="Rift|Audio|UI")
	void PlayButtonHoverSound();

	UFUNCTION(BlueprintCallable, Category="Rift|Audio|UI")
	void PlayButtonClickSound();

	UFUNCTION(BlueprintCallable, Category="Rift|Audio|UI")
	void PlayBackSound();

	UFUNCTION(BlueprintCallable, Category="Rift|Audio|UI")
	void PlayErrorSound();

private:
	const URiftAudioConfig* GetAudioConfig() const;
	void HandlePostLoadMapWithWorld(UWorld* LoadedWorld);
	void HandlePostWorldInitialization(UWorld* World, const UWorld::InitializationValues);
	void BindWorldBeginPlay(UWorld* World);
	void ClearWorldBeginPlayDelegate();
	void HandleWorldBeginPlay();
	void SetSoundClassVolume(USoundClass* SoundClass, float Volume);
	void ClearFrontendMusicComponent();

	UFUNCTION()
	void HandleFrontendMusicFinished();

	UPROPERTY(Transient)
	TObjectPtr<UAudioComponent> FrontendMusicComponent;

	FDelegateHandle PostLoadMapWithWorldDelegateHandle;
	FDelegateHandle PostWorldInitializationDelegateHandle;
	FDelegateHandle WorldBeginPlayDelegateHandle;
	TWeakObjectPtr<UWorld> BoundBeginPlayWorld;
	bool bFrontendMusicStopping = false;
};
