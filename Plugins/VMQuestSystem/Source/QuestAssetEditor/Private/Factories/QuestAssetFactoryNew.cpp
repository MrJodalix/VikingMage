// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "QuestAssetFactoryNew.h"

#include "QuestAsset/Public/QuestAsset.h"


/* UTextAssetFactoryNew structors
 *****************************************************************************/

UQuestAssetFactoryNew::UQuestAssetFactoryNew(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SupportedClass = UQuestAsset::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}


/* UFactory overrides
 *****************************************************************************/

UObject* UQuestAssetFactoryNew::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{	
	return NewObject<UQuestAsset>(InParent, InClass, InName, Flags);
}


bool UQuestAssetFactoryNew::ShouldShowInNewMenu() const
{
	return true;
}
