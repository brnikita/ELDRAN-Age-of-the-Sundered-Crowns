// Copyright KELDRAN. KeldranNet module (Docs/00_ImplementationSpec.md).
using UnrealBuildTool;

public class KeldranNet : ModuleRules
{
	public KeldranNet(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"HTTP",
			"Json",
			"JsonUtilities"
		});
	}
}
