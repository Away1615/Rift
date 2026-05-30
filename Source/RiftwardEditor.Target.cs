// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class RiftwardEditorTarget : TargetRules
{
	public RiftwardEditorTarget( TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_6;
		ExtraModuleNames.Add("Riftward");
	}
}
