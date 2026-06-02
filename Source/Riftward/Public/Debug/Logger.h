// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

DECLARE_LOG_CATEGORY_EXTERN(LogRiftward, Log, All);

/**
 *
 */
enum class ELogOutputType : uint8
{
	LogOnly,
	Screen
};

class RIFTWARD_API Logger
{
public:
	static void Log(
		const UObject* WorldContextObject,
		const FString& Message,
		const ELogOutputType Output = ELogOutputType::Screen,
		const float ScreenDuration = 3.0f
	);

	static void Error(
		const UObject* WorldContextObject,
		const FString& Message,
		const ELogOutputType Output = ELogOutputType::Screen,
		const float ScreenDuration = 5.0f
	);

private:
	static void AddScreenMessage(const FString& Message, const FColor& Color, const float Duration);

	static FString GetNetLabel(const UObject* WorldContextObject);
};
