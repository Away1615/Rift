// Fill out your copyright notice in the Description page of Project Settings.


#include "Debug/Logger.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"

DEFINE_LOG_CATEGORY(LogRift);

namespace
{
	// Keep in sync with ELogSystem: same order, one entry per value (excluding Count).
	const TCHAR* GLogSystemNames[] = {
		TEXT("General"),
		TEXT("Character"),
		TEXT("Weapon"),
		TEXT("Animation"),
		TEXT("Input"),
		TEXT("Camera"),
		TEXT("Ability"),
		TEXT("AI"),
		TEXT("Network"),
		TEXT("UI"),
	};

	static_assert(UE_ARRAY_COUNT(GLogSystemNames) == static_cast<uint32>(ELogSystem::Count),
		"GLogSystemNames must have exactly one entry per ELogSystem value (excluding Count). Update both together.");

	void HandleLogToggleCommand(const TArray<FString>& Args, bool bEnable)
	{
		ELogSystem System;
		if (Args.Num() == 0 || !FLogger::FindSystemByName(Args[0], System))
		{
			UE_LOG(LogRift, Warning,
				TEXT("[Logger] Usage: Rift.Log.%s <SystemName>  (General, Character, Weapon, Animation, Input, Camera, Ability, AI, Network, UI)"),
				bEnable ? TEXT("Enable") : TEXT("Disable"));
			return;
		}

		FLogger::SetSystemEnabled(System, bEnable);
		UE_LOG(LogRift, Log, TEXT("[Logger] %s log system '%s'"),
			bEnable ? TEXT("Enabled") : TEXT("Disabled"), *FLogger::GetSystemName(System));
	}

	void HandleLogListCommand(const TArray<FString>& /*Args*/)
	{
		UE_LOG(LogRift, Log, TEXT("[Logger] Log system states (toggle with Rift.Log.Enable / Rift.Log.Disable):"));

		for (uint32 Index = 0; Index < static_cast<uint32>(ELogSystem::Count); ++Index)
		{
			const ELogSystem System = static_cast<ELogSystem>(Index);
			UE_LOG(LogRift, Log, TEXT("  %-10s : %s"),
				*FLogger::GetSystemName(System), FLogger::IsSystemEnabled(System) ? TEXT("ON") : TEXT("off"));
		}
	}
}

// All systems start enabled - existing behaviour is unchanged until you opt to silence one.
uint32 FLogger::EnabledSystemsMask = ~0u;

void FLogger::Log(const UObject* WorldContextObject, const FString& Message, const ELogSystem System, const ELogOutputType Output, const float ScreenDuration)
{
	if (!IsSystemEnabled(System))
	{
		return;
	}

	const FString NetLabel = GetNetLabel(WorldContextObject);
	const FString FinalMessage = FString::Printf(TEXT("[%s][%s] %s"), *NetLabel, *GetSystemName(System), *Message);

	UE_LOG(LogRift, Log, TEXT("%s"), *FinalMessage);

	if (Output == ELogOutputType::Screen)
	{
		AddScreenMessage(FinalMessage, FColor::Green, ScreenDuration);
	}
}

void FLogger::Error(const UObject* WorldContextObject, const FString& Message, const ELogSystem System, const ELogOutputType Output, const float ScreenDuration)
{
	if (!IsSystemEnabled(System))
	{
		return;
	}

	const FString NetLabel = GetNetLabel(WorldContextObject);
	const FString FinalMessage = FString::Printf(TEXT("[%s][%s] %s"), *NetLabel, *GetSystemName(System), *Message);
	UE_LOG(LogRift, Error, TEXT("%s"), *FinalMessage);

	if (Output == ELogOutputType::Screen)
	{
		AddScreenMessage(FinalMessage, FColor::Red, ScreenDuration);
	}
}

void FLogger::SetSystemEnabled(const ELogSystem System, const bool bEnabled)
{
	const uint32 Bit = 1u << static_cast<uint32>(System);

	if (bEnabled)
	{
		EnabledSystemsMask |= Bit;
	}
	else
	{
		EnabledSystemsMask &= ~Bit;
	}
}

bool FLogger::IsSystemEnabled(const ELogSystem System)
{
	const uint32 Bit = 1u << static_cast<uint32>(System);
	return (EnabledSystemsMask & Bit) != 0;
}

FString FLogger::GetSystemName(const ELogSystem System)
{
	const uint32 Index = static_cast<uint32>(System);

	if (Index < UE_ARRAY_COUNT(GLogSystemNames))
	{
		return GLogSystemNames[Index];
	}

	return TEXT("Unknown");
}

bool FLogger::FindSystemByName(const FString& Name, ELogSystem& OutSystem)
{
	for (uint32 Index = 0; Index < UE_ARRAY_COUNT(GLogSystemNames); ++Index)
	{
		if (Name.Equals(GLogSystemNames[Index], ESearchCase::IgnoreCase))
		{
			OutSystem = static_cast<ELogSystem>(Index);
			return true;
		}
	}

	return false;
}

void FLogger::AddScreenMessage(const FString& Message, const FColor& Color, const float Duration)
{
#if !UE_BUILD_SHIPPING
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, Duration, Color, Message);
	}
#endif
}

FString FLogger::GetNetLabel(const UObject* WorldContextObject)
{
	const UWorld* World = WorldContextObject ? WorldContextObject->GetWorld() : nullptr;

	if (!World)
	{
		return TEXT("NoWorld");
	}

	switch (World->GetNetMode())
	{
	case NM_Standalone:
		return TEXT("Standalone");

	case NM_DedicatedServer:
	case NM_ListenServer:
		return TEXT("Server");

	case NM_Client:
#if WITH_EDITOR
		if (GEngine)
		{
			if (const FWorldContext* WorldContext = GEngine->GetWorldContextFromWorld(World))
			{
				return FString::Printf(TEXT("Client%d"), WorldContext->PIEInstance);
			}
		}
#endif
		return TEXT("Client");

	default:
		return TEXT("Unknown");
	}
}

// --- Console commands: quickly flip a system's debug output on/off while playing/debugging ---
// Usage in the in-game console (~):
//   Rift.Log.Enable Ability
//   Rift.Log.Disable Weapon
//   Rift.Log.List

static FAutoConsoleCommand CVarRiftLogEnable(
	TEXT("Rift.Log.Enable"),
	TEXT("Enable FLogger output for one debug system. Usage: Rift.Log.Enable <SystemName>"),
	FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
	{
		HandleLogToggleCommand(Args, true);
	})
);

static FAutoConsoleCommand CVarRiftLogDisable(
	TEXT("Rift.Log.Disable"),
	TEXT("Disable FLogger output for one debug system. Usage: Rift.Log.Disable <SystemName>"),
	FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
	{
		HandleLogToggleCommand(Args, false);
	})
);

static FAutoConsoleCommand CVarRiftLogList(
	TEXT("Rift.Log.List"),
	TEXT("List all FLogger debug systems and whether each is currently enabled."),
	FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
	{
		HandleLogListCommand(Args);
	})
);
