// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class QuestAsset : ModuleRules
{
	public QuestAsset(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        bEnforceIWYU = true;

        PublicIncludePaths.AddRange(
            new string[] {
                Path.Combine(ModuleDirectory, "Public")
                // ... add public include paths required here ...
            });


        PrivateIncludePaths.AddRange(
            new string[] {
                Path.Combine(ModuleDirectory, "Private")
                // ... add other private include paths required here ...
            });


		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				//"CoreUObject",
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
                // Core modules
                "Engine",
                "CoreUObject",
                "Projects", // IPluginManager
                "RenderCore",
                "InputCore",
                "SlateCore",
                "Slate",
              
                // e.g. FPlatformApplicationMisc::ClipboardCopy
                "ApplicationCore",
			}
			);

        // Add MessageLog support
        if (Target.bBuildDeveloperTools)
        {
            PrivateDependencyModuleNames.Add("MessageLog");
        }

        DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);

        if (Target.bBuildEditor)
        {
            
            PrivateDependencyModuleNames.Add("EditorWidgets");
            PrivateDependencyModuleNames.Add("UnrealEd");
            /*PrivateDependencyModuleNames.Add("DetailCustomizations");
            PrivateDependencyModuleNames.Add("PropertyEditor");
            PrivateDependencyModuleNames.Add("EditorStyle");


            // Used for the Blueprint Nodes
            PrivateDependencyModuleNames.Add("BlueprintGraph");
            PrivateDependencyModuleNames.Add("Kismet");
            PrivateDependencyModuleNames.Add("KismetCompiler");
            PrivateDependencyModuleNames.Add("KismetWidgets");

            //graph stuff
            PrivateDependencyModuleNames.Add("GraphEditor");
            PrivateDependencyModuleNames.Add("ContentBrowser");*/


            // Accessing the menu
            PrivateDependencyModuleNames.Add("WorkspaceMenuStructure");

        }
	}
}
