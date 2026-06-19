// Copyright KELDRAN. Primary game module build rules.
using UnrealBuildTool;

public class Keldran : ModuleRules
{
	public Keldran(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		// Primary game module depends on all domain modules so they compile & link into every
		// target (Docs/00_ImplementationSpec.md module map). Domain modules carry their own deps.
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"KeldranCore",
			"KeldranCharacter",
			"KeldranAbilities",
			"KeldranInventory",
			"KeldranQuests",
			"KeldranAI",
			"KeldranNet",
			"KeldranUI",
			"KeldranServer",
			"KeldranTests"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });
	}
}
