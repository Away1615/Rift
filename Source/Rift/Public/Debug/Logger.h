// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

DECLARE_LOG_CATEGORY_EXTERN(LogRift, Log, All);

enum class ELogOutputType : uint8
{
	LogOnly,
	Screen
};

// Per-system debug tag for FLogger; toggle at runtime via Rift.Log.Enable/Disable/List (Logger.cpp).
// Keep in sync with GLogSystemNames in Logger.cpp (same order, "Count" excluded).
enum class ELogSystem : uint8
{
	General,
	Character,
	Weapon,
	Animation,
	Input,
	Camera,
	Ability,
	AI,
	Network,
	UI,

	Count // sentinel - keep last, sizes the enabled-systems bitmask (not a real system)
};

class RIFT_API FLogger
{
public:
	static void Log(
		const UObject* WorldContextObject,
		const FString& Message,
		ELogSystem System = ELogSystem::General,
		const ELogOutputType Output = ELogOutputType::Screen,
		const float ScreenDuration = 3.0f
	);

	static void Error(
		const UObject* WorldContextObject,
		const FString& Message,
		ELogSystem System = ELogSystem::General,
		const ELogOutputType Output = ELogOutputType::Screen,
		const float ScreenDuration = 5.0f
	);

	static void SetSystemEnabled(ELogSystem System, bool bEnabled);
	static bool IsSystemEnabled(ELogSystem System);

	static FString GetSystemName(ELogSystem System);
	static bool FindSystemByName(const FString& Name, ELogSystem& OutSystem);

private:
	static void AddScreenMessage(const FString& Message, const FColor& Color, const float Duration);

	static FString GetNetLabel(const UObject* WorldContextObject);

	// Bitmask, bit (1 << System); all systems enabled by default.
	static uint32 EnabledSystemsMask;
};
