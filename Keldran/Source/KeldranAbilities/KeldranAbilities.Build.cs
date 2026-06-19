// Copyright KELDRAN. KeldranAbilities module (Docs/00_ImplementationSpec.md).
using UnrealBuildTool;

public class KeldranAbilities : ModuleRules
{
	public KeldranAbilities(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"GameplayAbilities",
			"GameplayTags",
			"GameplayTasks"
		});
	}
}
