#pragma once

#include "AssetTypeActions_Base.h"

//class ISlateStyle;

class FAssetTypeActions_QuestAsset : public FAssetTypeActions_Base
{
public:
	/**
	 * Creates and initializes a new instance.
	 *
	 * @param InStyle The style set to use for asset editor toolkits.
	 */
	//FAssetTypeActions_QuestAsset(const TSharedRef<ISlateStyle>& InStyle);
	FAssetTypeActions_QuestAsset();


public:

	//~ FAssetTypeActions_Base overrides

	virtual bool CanFilter() override;
	
	/**
	 * Get all actions the should be shown in the context menu.
	 */
	virtual void GetActions(const TArray<UObject*>& InObjects, FMenuBuilder& MenuBuilder) override;
	
	/**
	 * Get the categories, where the asset factory can be found on right clicking in the content browser.
	 */
	virtual uint32 GetCategories() override;
	virtual FText GetName() const override;
	virtual UClass* GetSupportedClass() const override;
	virtual FColor GetTypeColor() const override;
	virtual bool HasActions(const TArray<UObject*>& InObjects) const override;
		
	virtual void OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor = TSharedPtr<IToolkitHost>()) override;

//private:

	/** Pointer to the style set to use for toolkits. */
	//TSharedRef<ISlateStyle> Style;
};