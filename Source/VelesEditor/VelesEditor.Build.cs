// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class VelesEditor : ModuleRules
{
	public VelesEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;


		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"Veles",
				"Blutility",
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"InputCore",
				"EditorFramework",
				"EditorStyle",
				"UnrealEd",
				"LevelEditor",
				"InteractiveToolsFramework",
				"EditorInteractiveToolsFramework",
				"ToolMenus",
				"ToolWidgets",
				"Projects",
				"UMG",
				"UMGEditor",
				"Landscape",
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
	}
}
