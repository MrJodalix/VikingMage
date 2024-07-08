

#include "CollectObjective.h"


UObjective* UCollectObjective::Copy(UObject* Outer, UQuestAsset* NewParent)
{
	UCollectObjective* CopyObjective = NewObject<UCollectObjective>(Outer, UCollectObjective::StaticClass());
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

	//Collect Objective Specifics
	CopyObjective->CollectAmount = CollectAmount;
	CopyObjective->CollectItemID = CollectItemID;

	return CopyObjective;
}


void UCollectObjective::CheckCollectObjective(int ItemID, int Amount) {
	if (ItemID == CollectItemID && Amount == CollectAmount)
	{
		SetObjectiveCompleted();		
	}
}

FObjectiveData UCollectObjective::GetObjectiveData()
{
	FObjectiveData Data = Super::GetObjectiveData();

	Data.CollectAmount = CollectAmount;
	
	return Data;
}

void UCollectObjective::SetObjectiveData(FObjectiveData Data)
{
	Super::SetObjectiveData(Data);

	CollectAmount = Data.CollectAmount;
}


