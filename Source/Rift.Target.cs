// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class RiftTarget : TargetRules
{
	public RiftTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_6;
		ExtraModuleNames.Add("Rift");
	}
}
