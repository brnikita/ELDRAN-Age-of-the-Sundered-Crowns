// Copyright KELDRAN. KeldranUI module (Docs/00_ImplementationSpec.md).
using UnrealBuildTool;

public class KeldranUI : ModuleRules
{
	public KeldranUI(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"UMG",
			"GameplayAbilities",
			"GameplayTags",
			"KeldranAbilities",
			"Slate",
			"SlateCore"
		});
	}
}
