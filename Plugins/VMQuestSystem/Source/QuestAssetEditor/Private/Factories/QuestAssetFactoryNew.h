// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.
#pragma once

#include "Factories/Factory.h"
#include "UObject/ObjectMacros.h"

#include "QuestAssetFactoryNew.generated.h"


/**
 * Implements a factory for UQuestAsset objects. Creates a new QuestAsset instance.
 */
UCLASS()
class QUESTASSETEDITOR_API UQuestAssetFactoryNew : public UFactory
{
	GENERATED_BODY()

public:
	UQuestAssetFactoryNew(const FObjectInitializer& ObjectInitializer);
	
	//~ UFactory Interface
	/**
	 * Constructor
	 */
	virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;

	/** Should the Quest Asset be shown in the menu?*/
	virtual bool ShouldShowInNewMenu() const override;
};
