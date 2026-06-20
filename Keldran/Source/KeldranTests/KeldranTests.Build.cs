// Copyright KELDRAN. KeldranTests module (Docs/00_ImplementationSpec.md).
using UnrealBuildTool;

public class KeldranTests : ModuleRules
{
	public KeldranTests(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"KeldranInventory",
			"KeldranQuests"
		});
	}
}
