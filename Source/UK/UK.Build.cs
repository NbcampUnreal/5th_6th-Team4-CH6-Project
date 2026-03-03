// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class UK : ModuleRules
{
	public UK(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"NavigationSystem",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG", 
			"Slate",
			"SlateCore",
			"GameplayAbilities",
            "GameplayTags",
            "GameplayTasks",
            "AdvancedSessions",
            "AdvancedSteamSessions",
            "OnlineSubsystem",
            "OnlineSubsystemUtils",
            "OnlineSubsystemSteam",
			"Json",
			"JsonUtilities",
			"AssetRegistry"
        });
		
		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"UK",
			"UK/Variant_Platforming",
			"UK/Variant_Platforming/Animation",
			"UK/Variant_Combat",
			"UK/Variant_Combat/AI",
			"UK/Variant_Combat/Animation",
			"UK/Variant_Combat/Gameplay",
			"UK/Variant_Combat/Interfaces",
			"UK/Variant_Combat/UI",
			"UK/Variant_SideScrolling",
			"UK/Variant_SideScrolling/AI",
			"UK/Variant_SideScrolling/Gameplay",
			"UK/Variant_SideScrolling/Interfaces",
			"UK/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
