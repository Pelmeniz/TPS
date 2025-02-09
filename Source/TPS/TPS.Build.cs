// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class TPS : ModuleRules
{
	public TPS(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "UMG", "HeadMountedDisplay", "NavigationSystem", "AIModule", "Niagara", "PhysicsCore", "EnhancedInput" });

		PublicIncludePaths.AddRange(new string[]
		{
			"TPS",
			"TPS/Character",
			"TPS/FuncLibrary",
			"TPS/Game",
			"TPS/Weapon",
			"TPS/Weapon/Projectiles"
            });
    }
}
