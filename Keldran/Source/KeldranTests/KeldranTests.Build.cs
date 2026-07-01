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
			"EnhancedInput",
			"InputCore",
			"GameplayAbilities",
			"GameplayTags",
			"KeldranCore",
			"KeldranInventory",
			"KeldranQuests",
			"KeldranAbilities",
			"KeldranAI",
			"KeldranCharacter"
		});
	}
}
