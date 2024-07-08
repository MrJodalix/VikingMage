// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "IAssetTools.h"
#include "Containers/Array.h"
#include "ISettingsModule.h"
#include "ISettingsSection.h"
#include "QuestCommands.h"
#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"
#include "Templates/SharedPointer.h"
#include "Toolkits/AssetEditorToolkit.h"

#include "AssetTypeActions/AssetTypeActions_QuestAsset.h"



#define LOCTEXT_NAMESPACE "FQuestSystemEditorModule"

class FQuestSystemEditorModule: public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override
	{
		//TODO (Testen): Muss initialisiert werden! (Siehe TextAssetEditor)
		//Style = MakeShareable(new FTextAssetEditorStyle());
		FQuestCommands::Register();
		
		RegisterAssetTools();
	}
	virtual void ShutdownModule() override
	{
		UnregisterAssetTools();

		FQuestCommands::Unregister();
	}


protected:

	/** Registers asset tool actions. */
	void RegisterAssetTools()
	{
		IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
		
		RegisterAssetTypeAction(AssetTools, MakeShareable(new FAssetTypeActions_QuestAsset()));
	}

	/**
	 * Registers a single asset type action.
	 *
	 * @param AssetTools The asset tools object to register with.
	 * @param Action The asset type action to register.
	 */
	void RegisterAssetTypeAction(IAssetTools& AssetTools, TSharedRef<IAssetTypeActions> Action)
	{
		AssetTools.RegisterAssetTypeActions(Action);
		RegisteredAssetTypeActions.Add(Action);
	}


	/** Unregisters asset tool actions. */
	void UnregisterAssetTools()
	{
		FAssetToolsModule* AssetToolsModule = FModuleManager::GetModulePtr<FAssetToolsModule>("AssetTools");

		if (AssetToolsModule != nullptr)
		{
			IAssetTools& AssetTools = AssetToolsModule->Get();

			for (auto Action : RegisteredAssetTypeActions)
			{
				AssetTools.UnregisterAssetTypeActions(Action);
			}
		}

		RegisteredAssetTypeActions.Empty();
	}

private:
	
	/** The collection of registered asset type actions. */
	TArray<TSharedRef<IAssetTypeActions>> RegisteredAssetTypeActions;

	/** Holds the plug-ins style set. */
	TSharedPtr<ISlateStyle> Style;

	
};

IMPLEMENT_MODULE(FQuestSystemEditorModule, QuestAssetEditor)

#undef LOCTEXT_NAMESPACE