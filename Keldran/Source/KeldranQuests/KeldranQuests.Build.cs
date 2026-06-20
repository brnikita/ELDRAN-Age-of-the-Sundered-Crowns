// Copyright KELDRAN. KeldranQuests module (Docs/00_ImplementationSpec.md).
using UnrealBuildTool;

public class KeldranQuests : ModuleRules
{
	public KeldranQuests(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"NetCore",
			"GameplayTags",
			"KeldranCore",
			"KeldranInventory",
			"KeldranNet"
		});
	}
}
