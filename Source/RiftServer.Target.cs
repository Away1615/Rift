// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class RiftServerTarget : TargetRules
{
	public RiftServerTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Server;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_6;
		ExtraModuleNames.Add("Rift");
	}
}