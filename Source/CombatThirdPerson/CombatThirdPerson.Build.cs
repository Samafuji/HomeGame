// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class CombatThirdPerson : ModuleRules
{
	public CombatThirdPerson(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG", "Slate", "SlateCore"
        });

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"CombatThirdPerson",
			"CombatThirdPerson/Variant_Platforming",
			"CombatThirdPerson/Variant_Combat",
			"CombatThirdPerson/Variant_Combat/AI",
			"CombatThirdPerson/Variant_SideScrolling",
			"CombatThirdPerson/Variant_SideScrolling/Gameplay",
			"CombatThirdPerson/Variant_SideScrolling/AI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
