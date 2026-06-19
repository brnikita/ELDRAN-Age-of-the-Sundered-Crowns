// Copyright KELDRAN. Dedicated server build target (server-authoritative MMO).
using UnrealBuildTool;
using System.Collections.Generic;

public class KeldranServerTarget : TargetRules
{
	public KeldranServerTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Server;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("Keldran");
	}
}
