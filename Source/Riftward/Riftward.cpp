// Copyright Epic Games, Inc. All Rights Reserved.

#include "Riftward.h"

#include "GameplayTags/RiftGameplayTags.h"
#include "Modules/ModuleManager.h"

class FRiftwardModule : public FDefaultGameModuleImpl
{
public:
	virtual void StartupModule() override
	{
		FRiftGameplayTags::InitializeNativeTags();
	}
};

IMPLEMENT_PRIMARY_GAME_MODULE( FRiftwardModule, Riftward, "Riftward" );
