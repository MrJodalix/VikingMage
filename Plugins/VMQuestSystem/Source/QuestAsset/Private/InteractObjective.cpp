
#include "InteractObjective.h"

UObjective* UInteractObjective::Copy(UObject* Outer, UQuestAsset* NewParent)
{
	UInteractObjective* CopyObjective = NewObject<UInteractObjective>(Outer, UInteractObjective::StaticClass());
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
	CopyObjective->target = target;
	CopyObjective->InteractID = InteractID;

	return CopyObjective;
}


void UInteractObjective::CheckInteractObjective(AActor* Actor) {
	//wurde der ZielActor geladen? (befindet sich im aktuellen Level)
	if (this->target.IsValid() && 
		UKismetMathLibrary::EqualEqual_ObjectObject(Actor, this->target.Get())) {
		SetObjectiveCompleted();
	}	
}

void UInteractObjective::CheckInteractObjectiveWithID(FName TriggeredID)
{
	if(this->InteractID.IsEqual(TriggeredID, ENameCase::CaseSensitive))
	{
		SetObjectiveCompleted();
	}
}
