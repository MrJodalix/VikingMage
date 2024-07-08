#pragma once

#include "QuestAsset/Private/Objective.h"
#include "Kismet/KismetMathLibrary.h"

#include "LocationObjective.generated.h"


UCLASS(BlueprintType, hidecategories = (Object))
class QUESTASSET_API ULocationObjective
	: public UObjective
{
	GENERATED_BODY()
public:
	
	/** Returns the Type of this objective */
	FText GetObjectiveType() { return FText::FromString("Location"); }

	UObjective* Copy(UObject* Outer, UQuestAsset* NewParent) override;
	
#if WITH_EDITOR
	FString GetNodeTypeString() const override { return TEXT("Location"); }
#endif

	// Checks if this objective is a valid objective. Only derived classes of UObjective are valid Objectives. 
	virtual bool IsValid() const override { return true; }
	
	/**
	* Pr�ft, ob der get�tete Gegner dieses Objective abschlie�t	
	*/
	UFUNCTION(BlueprintCallable, Category="Quest|Locate")
	void CheckLocationObjective(AActor* ReachedLocation);

	/** Marker des zu erreichenden Orts (NUR LocationMarker ausw�hlen)*/
	//TODO: LocationMarker in C++ machen und dann hier als Typ w�hlen
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	TSoftObjectPtr<AActor> LocMarker;
};