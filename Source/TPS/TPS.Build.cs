// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class TPS : ModuleRules
{
	public TPS(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "NavigationSystem", "AIModule", "Niagara", "EnhancedInput" });

		PublicIncludePaths.AddRange(new string[]
		{
			"TPS",
			"TPS/Character",
			"TPS/FuncLibrary",
			"TPS/Game",
            "TPS/Weapons",
            "TPS/Weapons/Projectiles"
            });
    }
}
