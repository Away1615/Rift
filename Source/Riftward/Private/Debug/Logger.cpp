// Fill out your copyright notice in the Description page of Project Settings.


#include "Debug/Logger.h"
#include "Engine/Engine.h"

DEFINE_LOG_CATEGORY(LogRiftward);

void Logger::Log(const FString& Message, const ELogOutputType Output, const float ScreenDuration)
{
	UE_LOG(LogRiftward, Log, TEXT("%s"), *Message);

	if (Output == ELogOutputType::Screen)
	{
		AddScreenMessage(Message, FColor::Green, ScreenDuration);
	}
}

void Logger::Error(const FString& Message, const ELogOutputType Output, const float ScreenDuration)
{
	UE_LOG(LogRiftward, Error, TEXT("%s"), *Message);

	if (Output == ELogOutputType::Screen)
	{
		AddScreenMessage(Message, FColor::Red, ScreenDuration);
	}
}

void Logger::AddScreenMessage(const FString& Message, const FColor& Color, const float Duration)
{
#if !UE_BUILD_SHIPPING
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, Duration, Color, Message);
	}
#endif
}
