#pragma once

#include "QuestAsset/Private/Objective.h"
#include "KillObjective.generated.h"


UCLASS(BlueprintType, hidecategories = (Object))
class QUESTASSET_API UKillObjective
	: public UObjective
{
	GENERATED_BODY()
public:
	
	/** Returns the Type of this objective */
	FText GetObjectiveType() { return FText::FromString("Kill"); }

	UObjective* Copy(UObject*, UQuestAsset* NewParent) override;
	
#if WITH_EDITOR
	FString GetNodeTypeString() const override { return TEXT("Kill"); }
#endif
			
	// Checks if this objective is a valid objective. Only derived classes of UObjective are valid Objectives. 
	virtual bool IsValid() const override { return true; }

	virtual FObjectiveData GetObjectiveData() override;
	virtual void SetObjectiveData(FObjectiveData Data) override;
	
	/**
	* Prüft, ob der getötete Gegner dieses Objective abschließt
	*/
	UFUNCTION(BlueprintCallable, Category = "Quest|Kill")
	void CheckKillObjective(AActor* enemy);

	/** zu tötender Gegnertyp */
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	TSubclassOf<AActor> TargetClass;

	/** Anzahl zu tötender Gegner */
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	int TargetKillmount;

	/** Anzahl bereits getöteter Gegner dieses Typs */
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	int CurrKillCount;
};