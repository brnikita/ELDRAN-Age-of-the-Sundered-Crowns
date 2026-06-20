// Copyright KELDRAN. KeldranCharacter module (Docs/00_ImplementationSpec.md).
using UnrealBuildTool;

public class KeldranCharacter : ModuleRules
{
	public KeldranCharacter(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"EnhancedInput",
			"GameplayAbilities",
			"GameplayTags",
			"KeldranCore",
			"KeldranAbilities",
			"KeldranInventory",
			"KeldranQuests"
		});
	}
}
