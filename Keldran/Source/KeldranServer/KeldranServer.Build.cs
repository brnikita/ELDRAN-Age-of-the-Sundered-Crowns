// Copyright KELDRAN. KeldranServer module (Docs/00_ImplementationSpec.md).
using UnrealBuildTool;

public class KeldranServer : ModuleRules
{
	public KeldranServer(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"Json",
			"KeldranCore",
			"KeldranNet",
			"KeldranInventory",
			"KeldranQuests"
		});
	}
}
