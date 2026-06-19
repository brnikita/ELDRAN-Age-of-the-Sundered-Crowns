// Copyright KELDRAN. Game (client) build target.
using UnrealBuildTool;
using System.Collections.Generic;

public class KeldranTarget : TargetRules
{
	public KeldranTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("Keldran");
	}
}
