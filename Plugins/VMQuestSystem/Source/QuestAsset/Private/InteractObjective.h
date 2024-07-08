#pragma once

#include "QuestAsset/Private/Objective.h"
#include "Kismet/KismetMathLibrary.h"

#include "InteractObjective.generated.h"


UCLASS(BlueprintType, hidecategories = (Object))
class QUESTASSET_API UInteractObjective
	: public UObjective
{
	GENERATED_BODY()
public:
	
#if WITH_EDITOR
	FString GetNodeTypeString() const override { return TEXT("Interact"); }
#endif

	/** Returns the Type of this objective */
	FText GetObjectiveType() { return FText::FromString("Interact"); }

	UObjective* Copy(UObject* Outer, UQuestAsset* NewParent) override;

	// Checks if this objective is a valid objective. Only derived classes of UObjective are valid Objectives. 
	virtual bool IsValid() const override { return true; }

	/**
	* Checks if an Interaction with an actor completes this objective.
	*/
	UFUNCTION(BlueprintCallable, Category = "Quest|Interact")
		void CheckInteractObjective(AActor* Actor);

	/**
	* Checks if an Interaction with an actor completes this objective.
	*/
	UFUNCTION(BlueprintCallable, Category = "Quest|Interact")
		void CheckInteractObjectiveWithID(FName InteractID);

	/** Actor, mit dem zu Interagieren ist */
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
		TSoftObjectPtr<AActor> target;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
		FName InteractID;
};