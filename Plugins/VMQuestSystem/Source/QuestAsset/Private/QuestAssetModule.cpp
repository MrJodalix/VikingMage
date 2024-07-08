// Copyright Epic Games, Inc. All Rights Reserved.

#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"


#define LOCTEXT_NAMESPACE "FVMQuestSystemModule"

/**
 * Modulclass für the QuestAsset module. Needs to exist for a module.
 */
class FQuestAssetModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override 
	{
		// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	};
	virtual void ShutdownModule() override 
	{
		// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
		// we call this function before unloading the module.
	};
};

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FQuestAssetModule, QuestAsset)