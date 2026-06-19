// Copyright KELDRAN. KeldranCore module (Docs/00_ImplementationSpec.md).
using UnrealBuildTool;

public class KeldranCore : ModuleRules
{
	public KeldranCore(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"GameplayTags"
		});
	}
}
