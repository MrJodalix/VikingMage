
#include "LocationObjective.h"

UObjective* ULocationObjective::Copy(UObject* Outer, UQuestAsset* NewParent)
{
	ULocationObjective* CopyObjective = NewObject<ULocationObjective>(Outer, ULocationObjective::StaticClass());
	CopyObjective->ID = ID;
	CopyObjective->Title = FText::FromString(this->Title.ToString());
	CopyObjective->Description = FText::FromString(this->Description.ToString());
	CopyObjective->IsQuestEnding = IsQuestEnding;
	CopyObjective->State = State;
	CopyObjective->EntryCondition = EntryCondition;

	CopyObjective->NodeGUID = NodeGUID;

	//Can copied this way as this are not pointers
	CopyObjective->EnterEvents = EnterEvents;
	CopyObjective->ExitEvents = ExitEvents;
	CopyObjective->ParentQuest = NewParent;

	//Kill Objective Specifics
	CopyObjective->LocMarker = LocMarker;

	return CopyObjective;
}


void ULocationObjective::CheckLocationObjective(AActor* ReachedLocation) {
	//wurde der Marker geladen? (befindet sich im aktuellen Level)
	if (this->LocMarker.IsValid() && 
		UKismetMathLibrary::EqualEqual_ObjectObject(ReachedLocation, this->LocMarker.Get())) {
		SetObjectiveCompleted();
	}	
}
