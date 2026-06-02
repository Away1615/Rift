// Fill out your copyright notice in the Description page of Project Settings.


#include "Debug/Logger.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

DEFINE_LOG_CATEGORY(LogRiftward);

void Logger::Log(const UObject* WorldContextObject, const FString& Message, const ELogOutputType Output, const float ScreenDuration)
{
	const FString NetLabel = GetNetLabel(WorldContextObject);
	const FString FinalMessage = FString::Printf(TEXT("[%s] %s"), *NetLabel, *Message);

	UE_LOG(LogRiftward, Log, TEXT("%s"), *FinalMessage);

	if (Output == ELogOutputType::Screen)
	{
		AddScreenMessage(FinalMessage, FColor::Green, ScreenDuration);
	}
}

void Logger::Error(const UObject* WorldContextObject, const FString& Message, const ELogOutputType Output, const float ScreenDuration)
{
	const FString NetLabel = GetNetLabel(WorldContextObject);
	const FString FinalMessage = FString::Printf(TEXT("[%s] %s"), *NetLabel, *Message);
	UE_LOG(LogRiftward, Error, TEXT("%s"), *FinalMessage);

	if (Output == ELogOutputType::Screen)
	{
		AddScreenMessage(FinalMessage, FColor::Green, ScreenDuration);
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

FString Logger::GetNetLabel(const UObject* WorldContextObject)
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
