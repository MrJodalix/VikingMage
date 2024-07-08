#pragma once

#include "QuestAsset/Private/Objective.h"
#include "CollectObjective.generated.h"


UCLASS(BlueprintType, hidecategories = (Object))
class QUESTASSET_API UCollectObjective
	: public UObjective
{
	GENERATED_BODY()
public:
	
	/** Returns the Type of this objective */
	FText GetObjectiveType() { return FText::FromString("Collect"); }

	UObjective* Copy(UObject* Outer, UQuestAsset* NewParent) override;
	
#if WITH_EDITOR
	FString GetNodeTypeString() const override { return TEXT("Collect"); }
#endif

	// Checks if this objective is a valid objective. Only derived classes of UObjective are valid Objectives. 
	virtual bool IsValid() const override { return true; }

	virtual FObjectiveData GetObjectiveData() override;

	virtual void SetObjectiveData(FObjectiveData Data) override;

	/**
	* Pr�ft, ob der get�tete Gegner dieses Objective abschlie�t
	*/
	UFUNCTION(BlueprintCallable, Category = "Quest|Collect")
	void CheckCollectObjective(int ItemID, int Amount);

	/** Item, das einzusammeln ist */
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	int CollectItemID;

	/** Anzahl zu sammelnder Gegenst�nde */
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	int CollectAmount;
};