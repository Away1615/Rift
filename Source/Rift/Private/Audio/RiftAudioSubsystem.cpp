#include "Audio/RiftAudioSubsystem.h"

#include "Components/AudioComponent.h"
#include "Core/RiftGameInstance.h"
#include "Data/Audio/RiftAudioConfig.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundClass.h"
#include "Sound/SoundMix.h"
#include "UObject/UObjectGlobals.h"

void URiftAudioSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	PostLoadMapWithWorldDelegateHandle = FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(
		this,
		&URiftAudioSubsystem::HandlePostLoadMapWithWorld
	);
	PostWorldInitializationDelegateHandle = FWorldDelegates::OnPostWorldInitialization.AddUObject(
		this,
		&URiftAudioSubsystem::HandlePostWorldInitialization
	);

	ApplyDefaultSoundMix();
	BindWorldBeginPlay(GetWorld());
	RefreshMusicForWorld(GetWorld());
}

void URiftAudioSubsystem::Deinitialize()
{
	if (PostLoadMapWithWorldDelegateHandle.IsValid())
	{
		FCoreUObjectDelegates::PostLoadMapWithWorld.Remove(PostLoadMapWithWorldDelegateHandle);
		PostLoadMapWithWorldDelegateHandle.Reset();
	}

	if (PostWorldInitializationDelegateHandle.IsValid())
	{
		FWorldDelegates::OnPostWorldInitialization.Remove(PostWorldInitializationDelegateHandle);
		PostWorldInitializationDelegateHandle.Reset();
	}

	ClearWorldBeginPlayDelegate();
	StopFrontendMusic(0.0f);

	Super::Deinitialize();
}

void URiftAudioSubsystem::RefreshMusicForWorld(UWorld* World)
{
	if (!World || !World->IsGameWorld())
	{
		return;
	}

	const URiftAudioConfig* AudioConfig = GetAudioConfig();
	if (!AudioConfig)
	{
		return;
	}

	const FString WorldPackageName = World->GetOutermost()->GetName();
	if (AudioConfig->IsFrontendMap(WorldPackageName))
	{
		PlayFrontendMusic();
		return;
	}

	if (AudioConfig->bStopFrontendMusicOutsideFrontendMaps)
	{
		StopFrontendMusic();
	}
}

void URiftAudioSubsystem::PlayFrontendMusic()
{
	const URiftAudioConfig* AudioConfig = GetAudioConfig();
	if (!AudioConfig || !AudioConfig->FrontendMusic)
	{
		return;
	}

	if (FrontendMusicComponent &&
		FrontendMusicComponent->IsPlaying() &&
		FrontendMusicComponent->GetSound() == AudioConfig->FrontendMusic &&
		!bFrontendMusicStopping)
	{
		return;
	}

	ClearFrontendMusicComponent();

	FrontendMusicComponent = UGameplayStatics::SpawnSound2D(
		this,
		AudioConfig->FrontendMusic,
		AudioConfig->FrontendMusicVolume,
		AudioConfig->FrontendMusicPitch,
		0.0f,
		nullptr,
		true,
		false
	);
	bFrontendMusicStopping = false;

	if (FrontendMusicComponent)
	{
		FrontendMusicComponent->OnAudioFinished.AddUniqueDynamic(
			this,
			&URiftAudioSubsystem::HandleFrontendMusicFinished
		);
	}
}

void URiftAudioSubsystem::StopFrontendMusic(const float FadeOutDuration)
{
	if (!FrontendMusicComponent)
	{
		bFrontendMusicStopping = false;
		return;
	}

	const URiftAudioConfig* AudioConfig = GetAudioConfig();
	const float EffectiveFadeOutDuration = FadeOutDuration >= 0.0f
		? FadeOutDuration
		: (AudioConfig ? AudioConfig->FrontendMusicFadeOutDuration : 0.0f);

	if (FrontendMusicComponent->IsPlaying() && EffectiveFadeOutDuration > 0.0f)
	{
		bFrontendMusicStopping = true;
		FrontendMusicComponent->FadeOut(EffectiveFadeOutDuration, 0.0f);
		return;
	}

	ClearFrontendMusicComponent();
}

bool URiftAudioSubsystem::IsFrontendMusicPlaying() const
{
	return FrontendMusicComponent && FrontendMusicComponent->IsPlaying() && !bFrontendMusicStopping;
}

void URiftAudioSubsystem::ApplyDefaultSoundMix()
{
	const URiftAudioConfig* AudioConfig = GetAudioConfig();
	if (!AudioConfig || !AudioConfig->DefaultSoundMix)
	{
		return;
	}

	UGameplayStatics::PushSoundMixModifier(this, AudioConfig->DefaultSoundMix);
}

void URiftAudioSubsystem::SetMasterVolume(const float Volume)
{
	const URiftAudioConfig* AudioConfig = GetAudioConfig();
	if (!AudioConfig)
	{
		return;
	}

	SetSoundClassVolume(AudioConfig->MasterSoundClass, Volume);
}

void URiftAudioSubsystem::SetMusicVolume(const float Volume)
{
	const URiftAudioConfig* AudioConfig = GetAudioConfig();
	if (!AudioConfig)
	{
		return;
	}

	SetSoundClassVolume(AudioConfig->MusicSoundClass, Volume);
}

void URiftAudioSubsystem::SetSFXVolume(const float Volume)
{
	const URiftAudioConfig* AudioConfig = GetAudioConfig();
	if (!AudioConfig)
	{
		return;
	}

	SetSoundClassVolume(AudioConfig->SFXSoundClass, Volume);
}

void URiftAudioSubsystem::SetUIVolume(const float Volume)
{
	const URiftAudioConfig* AudioConfig = GetAudioConfig();
	if (!AudioConfig)
	{
		return;
	}

	SetSoundClassVolume(AudioConfig->UISoundClass, Volume);
}

void URiftAudioSubsystem::SetAmbienceVolume(const float Volume)
{
	const URiftAudioConfig* AudioConfig = GetAudioConfig();
	if (!AudioConfig)
	{
		return;
	}

	SetSoundClassVolume(AudioConfig->AmbienceSoundClass, Volume);
}

void URiftAudioSubsystem::PlayUISound(const ERiftUISoundType SoundType)
{
	const URiftAudioConfig* AudioConfig = GetAudioConfig();
	if (!AudioConfig)
	{
		return;
	}

	USoundBase* UISound = AudioConfig->GetUISound(SoundType);
	if (!UISound)
	{
		return;
	}

	UGameplayStatics::PlaySound2D(
		this,
		UISound,
		AudioConfig->UISoundVolume,
		AudioConfig->UISoundPitch
	);
}

void URiftAudioSubsystem::PlayButtonHoverSound()
{
	PlayUISound(ERiftUISoundType::ButtonHover);
}

void URiftAudioSubsystem::PlayButtonClickSound()
{
	PlayUISound(ERiftUISoundType::ButtonClick);
}

void URiftAudioSubsystem::PlayBackSound()
{
	PlayUISound(ERiftUISoundType::Back);
}

void URiftAudioSubsystem::PlayErrorSound()
{
	PlayUISound(ERiftUISoundType::Error);
}

const URiftAudioConfig* URiftAudioSubsystem::GetAudioConfig() const
{
	const URiftGameInstance* RiftGameInstance = Cast<URiftGameInstance>(GetGameInstance());
	return RiftGameInstance ? RiftGameInstance->AudioConfig : nullptr;
}

void URiftAudioSubsystem::HandlePostLoadMapWithWorld(UWorld* LoadedWorld)
{
	BindWorldBeginPlay(LoadedWorld);
	RefreshMusicForWorld(LoadedWorld);
}

void URiftAudioSubsystem::HandlePostWorldInitialization(
	UWorld* World,
	const UWorld::InitializationValues
)
{
	BindWorldBeginPlay(World);
}

void URiftAudioSubsystem::BindWorldBeginPlay(UWorld* World)
{
	if (!World || !World->IsGameWorld() || BoundBeginPlayWorld.Get() == World)
	{
		return;
	}

	ClearWorldBeginPlayDelegate();

	BoundBeginPlayWorld = World;
	WorldBeginPlayDelegateHandle = World->OnWorldBeginPlay.AddUObject(
		this,
		&URiftAudioSubsystem::HandleWorldBeginPlay
	);
}

void URiftAudioSubsystem::ClearWorldBeginPlayDelegate()
{
	if (WorldBeginPlayDelegateHandle.IsValid())
	{
		if (UWorld* BoundWorld = BoundBeginPlayWorld.Get())
		{
			BoundWorld->OnWorldBeginPlay.Remove(WorldBeginPlayDelegateHandle);
		}

		WorldBeginPlayDelegateHandle.Reset();
	}

	BoundBeginPlayWorld.Reset();
}

void URiftAudioSubsystem::HandleWorldBeginPlay()
{
	RefreshMusicForWorld(BoundBeginPlayWorld.Get());
}

void URiftAudioSubsystem::SetSoundClassVolume(USoundClass* SoundClass, const float Volume)
{
	const URiftAudioConfig* AudioConfig = GetAudioConfig();
	if (!AudioConfig || !AudioConfig->DefaultSoundMix || !SoundClass)
	{
		return;
	}

	UGameplayStatics::SetSoundMixClassOverride(
		this,
		AudioConfig->DefaultSoundMix,
		SoundClass,
		FMath::Max(0.0f, Volume),
		1.0f,
		0.05f,
		true
	);
	UGameplayStatics::PushSoundMixModifier(this, AudioConfig->DefaultSoundMix);
}

void URiftAudioSubsystem::ClearFrontendMusicComponent()
{
	if (!FrontendMusicComponent)
	{
		bFrontendMusicStopping = false;
		return;
	}

	FrontendMusicComponent->OnAudioFinished.RemoveDynamic(
		this,
		&URiftAudioSubsystem::HandleFrontendMusicFinished
	);
	FrontendMusicComponent->Stop();
	FrontendMusicComponent = nullptr;
	bFrontendMusicStopping = false;
}

void URiftAudioSubsystem::HandleFrontendMusicFinished()
{
	FrontendMusicComponent = nullptr;
	bFrontendMusicStopping = false;
}
