// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class QuestAssetEditor : ModuleRules
{
	public QuestAssetEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        bEnforceIWYU = true;

        DynamicallyLoadedModuleNames.AddRange(
			new string[] {
				//"AssetTools",
				//"MainFrame",
//				"WorkspaceMenuStructure",
			});

        PublicIncludePaths.AddRange(
            new string[] {
               // Path.Combine(ModuleDirectory, "Public")
                // ... add public include paths required here ...
            });


        string PrivateDirectory = Path.Combine(ModuleDirectory, "Private");
        PrivateIncludePaths.AddRange(
            new string[] {
                PrivateDirectory,
                "QuestAssetEditor/Private/AssetTypeActions",
                "QuestAssetEditor/Private/Factories",
                "QuestAssetEditor/Private/Styles",
                "QuestAssetEditor/Private/Toolkits",
                "QuestAssetEditor/Private/Toolkits/Nodes",
                "QuestAssetEditor/Private/Toolkits/Graph",
                //Path.Combine(PrivateDirectory, "Commandlets")
                // ... add other private include paths required here ...
            });

        PublicDependencyModuleNames.AddRange(
            new string[] {
                "Core",
                "QuestAsset",
                // ... add other public dependencies that you statically link with here ...
            });

        PrivateDependencyModuleNames.AddRange(
            new string[] {
                // Core modules
                "Engine",
                "CoreUObject",
                "Projects", // IPluginManager
                "UnrealEd", // for FAssetEditorManager
                "RenderCore",
                "InputCore",
                "SlateCore",
                "Slate",
                "EditorStyle",
                "MessageLog",
                "EditorWidgets",

                // Accessing the menu
                "WorkspaceMenuStructure",

                // Details/Property editor
                "DetailCustomizations",
                "PropertyEditor",

                // Used for the Blueprint Nodes
                "BlueprintGraph",
                "Kismet",
                "KismetCompiler",
                "KismetWidgets",

                // graph stuff
                "GraphEditor",
                "ContentBrowser",

                // e.g. FPlatformApplicationMisc::ClipboardCopy
                "ApplicationCore",
                
            });

#if UE_4_24_OR_LATER
        PrivateDependencyModuleNames.Add("ToolMenus");
#endif
#if UE_4_26_OR_LATER
        PrivateDependencyModuleNames.Add("ContentBrowserData");
#endif
#if UE_5_0_OR_LATER
		PrivateDependencyModuleNames.Add("EditorFramework");
#endif

        PrivateIncludePathModuleNames.AddRange(
            new string[] {
                "AssetRegistry",
                "AssetTools",
            });



    }
}
