

#include "KillObjective.h"


UObjective* UKillObjective::Copy(UObject* Outer, UQuestAsset* NewParent)
{
	UKillObjective* CopyObjective = NewObject<UKillObjective>(Outer, UKillObjective::StaticClass());
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
	CopyObjective->TargetClass = TargetClass;
	CopyObjective->CurrKillCount = CurrKillCount;
	CopyObjective->TargetKillmount = TargetKillmount;
	
	return CopyObjective;
}


void UKillObjective::CheckKillObjective(AActor* enemy) {

	//Count up
	if (enemy->GetClass()->IsChildOf(TargetClass)) {
		CurrKillCount++;				
	}

	if(CurrKillCount >= TargetKillmount)
	{
		SetObjectiveCompleted();		
	}	
}

FObjectiveData UKillObjective::GetObjectiveData()
{
	FObjectiveData Data = Super::GetObjectiveData();
	
	Data.CurrKillCount = CurrKillCount;

	return Data;
}

void UKillObjective::SetObjectiveData(FObjectiveData Data)
{
	Super::SetObjectiveData(Data);

	CurrKillCount = Data.CurrKillCount;	
}


