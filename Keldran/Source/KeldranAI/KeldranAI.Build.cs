// Copyright KELDRAN. KeldranAI module (Docs/00_ImplementationSpec.md).
using UnrealBuildTool;

public class KeldranAI : ModuleRules
{
	public KeldranAI(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"AIModule",
			"GameplayTasks"
		});
	}
}
