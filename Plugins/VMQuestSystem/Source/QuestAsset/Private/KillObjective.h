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
	* Pr�ft, ob der get�tete Gegner dieses Objective abschlie�t
	*/
	UFUNCTION(BlueprintCallable, Category = "Quest|Kill")
	void CheckKillObjective(AActor* enemy);

	/** zu t�tender Gegnertyp */
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	TSubclassOf<AActor> TargetClass;

	/** Anzahl zu t�tender Gegner */
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	int TargetKillmount;

	/** Anzahl bereits get�teter Gegner dieses Typs */
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	int CurrKillCount;
};