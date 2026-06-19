// Copyright KELDRAN. Primary game module build rules.
using UnrealBuildTool;

public class Keldran : ModuleRules
{
	public Keldran(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		// Kept minimal for a clean first compile. GAS/UMG/AI deps are added in M1-2/M2 as the
		// KeldranCore/Abilities/etc. modules come online (Docs/00_ImplementationSpec.md).
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });
	}
}
