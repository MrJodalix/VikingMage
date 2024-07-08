#include "AssetTypeActions_QuestAsset.h"

#include "QuestAsset.h"

#include "Toolkits/QuestAssetEditorToolkit.h"

#define LOCTEXT_NAMESPACE "AssetTypeActions"

/* FTextAssetActions constructors
 *****************************************************************************/

//This constructor takes a Style, used for a custom editor menu. Checkout TextAsset for details.
/*FAssetTypeActions_QuestAsset::FAssetTypeActions_QuestAsset(const TSharedRef<ISlateStyle>& InStyle)
	: Style(InStyle)
{ }*/
FAssetTypeActions_QuestAsset::FAssetTypeActions_QuestAsset()
{ }


/* FAssetTypeActions_Base overrides
 *****************************************************************************/

bool FAssetTypeActions_QuestAsset::CanFilter()
{
	return true;
}

void FAssetTypeActions_QuestAsset::GetActions(const TArray<UObject*>& InObjects, FMenuBuilder& MenuBuilder)
{
	FAssetTypeActions_Base::GetActions(InObjects, MenuBuilder);

	//TODO: Add further actions to the context menu on right clicking on the asset
}

uint32 FAssetTypeActions_QuestAsset::GetCategories()
{
	return EAssetTypeCategories::Gameplay;
}


FText FAssetTypeActions_QuestAsset::GetName() const
{
	return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_TextAsset", "Quest Asset");
}


UClass* FAssetTypeActions_QuestAsset::GetSupportedClass() const
{
	return UQuestAsset::StaticClass();
}


FColor FAssetTypeActions_QuestAsset::GetTypeColor() const
{
	return FColor::Turquoise;
}


bool FAssetTypeActions_QuestAsset::HasActions(const TArray<UObject*>& InObjects) const
{
	return true;
}

void FAssetTypeActions_QuestAsset::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor)
{		
	EToolkitMode::Type Mode = EditWithinLevelEditor.IsValid()
		? EToolkitMode::WorldCentric
		: EToolkitMode::Standalone;

	for (auto ObjIt = InObjects.CreateConstIterator(); ObjIt; ++ObjIt)
	{
		auto QuestAsset = Cast<UQuestAsset>(*ObjIt);

		if (QuestAsset != nullptr)
		{
			//Create Toolkit (Toolkit = Editor Window)
			TSharedRef<FQuestAssetEditorToolkit> EditorToolkit = MakeShareable(new FQuestAssetEditorToolkit());
			//Initialize Toolkit
			EditorToolkit->Initialize(QuestAsset, Mode, EditWithinLevelEditor);
		}
	}
}


#undef LOCTEXT_NAMESPACE
