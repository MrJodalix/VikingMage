//TODO: Copyright

#include "Objective.h"

#include "QuestAsset.h"

#if WITH_EDITOR
void UObjective::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Signal to the listeners
	OnObjectivePropertyChanged.Broadcast(PropertyChangedEvent);
	BroadcastPropertyEdgeIndexChanged = INDEX_NONE;
}

void UObjective::PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent)
{
	// The Super::PostEditChangeChainProperty will construct a new FPropertyChangedEvent that will only have the Property and the
// MemberProperty name and it will call the PostEditChangeProperty, so we must get the array index of the Nodes modified from here.
// If you want to preserve all the change history of the tree you must broadcast the event from here to the children, but be warned
// that Property and MemberProperty are not set properly.
	BroadcastPropertyEdgeIndexChanged = PropertyChangedEvent.GetArrayIndex(GET_MEMBER_NAME_STRING_CHECKED(UObjective, Children));
	Super::PostEditChangeChainProperty(PropertyChangedEvent);
}


#endif //#if WITH_EDITOR

bool UObjective::HandleNodeEnter()
{
	FireEvents(EnterEvents, true);

	//TODO: Why bool?
	return true;
}

void UObjective::FireEvents(TArray<FQuestEvent> Events, bool IsEnter)
{
	for (const FQuestEvent& Event : Events)
	{
		if (IsEnter)
		{
			Event.CallEnter();
		}
		else
		{
			Event.CallExit();
		}
	}
}

FObjectiveData UObjective::GetObjectiveData()
{
	FObjectiveData Data;
	Data.Title = Title;
	Data.State = State;
	
	return Data;
}

void UObjective::SetObjectiveData(FObjectiveData Data)
{
	Title = Data.Title;
	State = Data.State;
}

bool UObjective::HandleNodeExit()
{
	FireEvents(ExitEvents, false);

	if (IsQuestEnding)
	{
		ParentQuest->IsComplete = true;
	}

	return true;
}

bool UObjective::CheckObjectiveActiveConditions()
{
	switch (EntryCondition)
	{
	case AND:
	{
		//All Parents completed?
		for (UObjective*& Objective : Parents)
		{
			if (Objective->State != EObjectiveState::COMPLETE)
			{
				return false;
			}
		}
		return true;
	}
	case OR:
	case EXOR:
	{
		// At least one Parent complete?
		for (UObjective*& Objective : Parents)
		{
			if (Objective->State == EObjectiveState::COMPLETE)
			{
				return true;
			}
		}
		return false;
	}

	default:
		return true;
	}
}

void UObjective::EXORParents()
{
	for (UObjective* Parent : Parents)
	{
		if (Parent->State != COMPLETE)
		{
			Parent->State = FAILED;
		}
		Parent->EXORParents();
	}
}


void UObjective::UpdateChildren()
{
	for (UObjective* Child : Children)
	{
		Child->UpdateVisibility();
	}
}

void UObjective::UpdateVisibility()
{
	if ((State == SOLVABLE || State == HIDDEN) && CheckObjectiveActiveConditions())
	{
		if (EntryCondition == EXOR)
		{
			EXORParents();
		}
		SetObjectiveVisible();
	}
}

