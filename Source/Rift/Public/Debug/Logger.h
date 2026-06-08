// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

DECLARE_LOG_CATEGORY_EXTERN(LogRift, Log, All);

/**
 *
 */
enum class ELogOutputType : uint8
{
	LogOnly,
	Screen
};

/**
 * Coarse-grained gameplay system tag attached to FLogger calls.
 *
 * Lets you silence or re-enable a single noisy system's debug output at runtime
 * without touching code or recompiling - useful when you only care about, say,
 * Ability logs right now and don't want Weapon/Animation spam in the way.
 *
 * Toggle at runtime via console commands (registered in Logger.cpp):
 *   Rift.Log.Enable  <SystemName>
 *   Rift.Log.Disable <SystemName>
 *   Rift.Log.List
 *
 * NOTE: Keep this in sync with GLogSystemNames in Logger.cpp (same order,
 * one entry per value, "Count" excluded). Append new systems before "Count".
 */
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

	// --- Runtime system toggles, intended to be driven by the console commands above ---

	static void SetSystemEnabled(ELogSystem System, bool bEnabled);
	static bool IsSystemEnabled(ELogSystem System);

	static FString GetSystemName(ELogSystem System);
	static bool FindSystemByName(const FString& Name, ELogSystem& OutSystem);

private:
	static void AddScreenMessage(const FString& Message, const FColor& Color, const float Duration);

	static FString GetNetLabel(const UObject* WorldContextObject);

	// Bit (1 << System) tells whether that system's output is currently enabled.
	// All systems start enabled, so existing behaviour is unchanged until you
	// explicitly silence one while debugging.
	static uint32 EnabledSystemsMask;
};
