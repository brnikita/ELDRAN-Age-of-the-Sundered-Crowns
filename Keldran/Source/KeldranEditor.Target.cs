// Copyright KELDRAN. Editor build target.
using UnrealBuildTool;
using System.Collections.Generic;

public class KeldranEditorTarget : TargetRules
{
	public KeldranEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("Keldran");
	}
}
