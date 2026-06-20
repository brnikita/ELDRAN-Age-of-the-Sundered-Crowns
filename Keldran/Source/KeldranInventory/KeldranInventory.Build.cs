// Copyright KELDRAN. KeldranInventory module (Docs/00_ImplementationSpec.md).
using UnrealBuildTool;

public class KeldranInventory : ModuleRules
{
	public KeldranInventory(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"NetCore",
			"GameplayTags",
			"GameplayAbilities",
			"KeldranCore"
		});
	}
}
