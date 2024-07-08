
#include "QuestAsset.h"

#include "CollectObjective.h"
#include "InteractObjective.h"
#include "KillObjective.h"
#include "LocationObjective.h"
#include "SaveGameSystem.h"
#include "Misc/FileHelper.h"
#include "Serialization/BufferArchive.h"


/* ---- Quest Function ----- */

FQuestData UQuestAsset::GetQuestData()
{
	FQuestData Data;	
	Data.IsComplete = IsComplete;

	for (UObjective* Objective : Objectives)
	{
		Data.ObjectiveData.Add(Objective->GetObjectiveData());
	}
	
	return Data;	
}

void UQuestAsset::SetQuestData(FQuestData Data)
{
	IsComplete = Data.IsComplete;

	check(Data.ObjectiveData.Num() == Objectives.Num());
	for(int i = 0; i < Data.ObjectiveData.Num(); i++)
	{
		Objectives[i]->SetObjectiveData(Data.ObjectiveData[i]);
	}
	
}


UQuestAsset* UQuestAsset::CreateInstanceOf(UObject* Outer)
{
	UE_LOG(LogTemp, Warning, TEXT("Start Creating Instance Of QuestAsset"));
	UQuestAsset* NewAsset = NewObject<UQuestAsset>(Outer, UQuestAsset::StaticClass());
	UE_LOG(LogTemp, Warning, TEXT("New Asset created"));

	NewAsset->AssetPath = this->AssetPath;
	
	NewAsset->Title = FText::FromString(Title.ToString());
	NewAsset->Description = FText::FromString(Description.ToString());
	NewAsset->ID = this->ID;
	NewAsset->IsComplete = this->IsComplete;
	NewAsset->IsStoryQuest = this->IsStoryQuest;
	//NewAsset->QuestGraph = this->QuestGraph;


	NewAsset->Objectives.Empty();
	/* OLD -> COPY*/
	TMap<UObjective*, UObjective*>ObjectiveMap;
	
	for (UObjective* Objective : this->Objectives)
	{
		
		UObjective* CopiedObjective = Objective->Copy(NewAsset, NewAsset);		
		NewAsset->AddNode(CopiedObjective);
		ObjectiveMap.Add(Objective, CopiedObjective);
	}

	/* Map: ALT -> NEU*/
	for (UObjective* Objective : this->Objectives)
	{
		UObjective* CopiedObjective = *ObjectiveMap.Find(Objective);

		//Children
		for(UObjective* ChildObjective : Objective->GetNodeChildren())
		{
			UObjective* CopyChildObjective = *ObjectiveMap.Find(ChildObjective);
			CopiedObjective->AddNodeChild(CopyChildObjective);
		}
		
		//Parents
		for (UObjective* ParentObjective : Objective->GetNodeParents())
		{
			UObjective* CopyParentObjective = *ObjectiveMap.Find(ParentObjective);
			CopiedObjective->AddNodeParent(CopyParentObjective);
		}


		//TODO: Copy DisableOnCompletion
		
	}
	//Set Start Node from Copied Objectives. Objectives must never be empty!
	NewAsset->StartNode = NewAsset->GetObjectives()[0];
		
	return NewAsset;
}

void UQuestAsset::EnemyKilled(AActor* Enemy)
{
	for (UObjective* Objective : GetActiveSolvableObjectives())
	{
		UKillObjective* KillObj = Cast<UKillObjective>(Objective);
		if (KillObj)
		{
			KillObj->CheckKillObjective(Enemy);
		}
	}
}

void UQuestAsset::ItemCollected(int ItemID, int ItemAmount)
{
	for (UObjective* Objective : GetActiveSolvableObjectives())
	{
		UCollectObjective* CollObj = Cast<UCollectObjective>(Objective);
		if (CollObj)
		{
			CollObj->CheckCollectObjective(ItemID, ItemAmount);
		}
	}
}

void UQuestAsset::LocationVisited(AActor* Location)
{
	for (UObjective* Objective : GetActiveSolvableObjectives())
	{
		ULocationObjective* LocObj = Cast<ULocationObjective>(Objective);
		if (LocObj)
		{
			LocObj->CheckLocationObjective(Location);
		}
	}
}

void UQuestAsset::InteractedWith(AActor* Interactor)
{
	for (UObjective* Objective : GetActiveSolvableObjectives())
	{
		UInteractObjective* InterObj = Cast<UInteractObjective>(Objective);
		if (InterObj)
		{
			InterObj->CheckInteractObjective(Interactor);
		}
	}
}

void UQuestAsset::TriggeredInteractionID(FName InteractionID)
{
	for (UObjective* Objective : GetActiveSolvableObjectives())
	{
		UInteractObjective* InterObj = Cast<UInteractObjective>(Objective);
		if (InterObj)
		{
			InterObj->CheckInteractObjectiveWithID(InteractionID);
		}
	}
}

bool UQuestAsset::IsObjectiveVisible(FName ObjectiveID)
{
	for (UObjective* Objective : GetVisibleObjectives())
	{
		if(Objective->ID.IsEqual(ObjectiveID))
		{
			return true;
		}
	}
	return false;	
}

bool UQuestAsset::IsObjectiveComplete(FName ObjectiveID)
{
	for (UObjective* Objective : GetObjectives())
	{
		if (Objective->State == COMPLETE && Objective->ID.IsEqual(ObjectiveID))
		{
			return true;
		}
	}
	return false;
}

bool UQuestAsset::ContainsObjective(FName ObjectiveID)
{
	for (UObjective* Objective : GetObjectives())
	{
		if (Objective->ID.IsEqual(ObjectiveID))
		{
			return true;
		}
	}
	return false;
}

/* ---- UObject interface ---- */
void UQuestAsset::PreSave(const ITargetPlatform* TargetPlatform)
{
	Super::PreSave(TargetPlatform);
	ID = GetQuestFName();
	bWasLoaded = true;
	//TODO (Checked): brauchen wir das?
}

void UQuestAsset::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);
	//TODO (Checked): Brauchen wir das? (
}

void UQuestAsset::PostLoad()
{
	Super::PostLoad();
	//TODO (Checked): Reicht das?

	if (!HasGUID())
	{
		RegenerateGUID();
	}


}

void UQuestAsset::PostInitProperties()
{
	Super::PostInitProperties();

	if (HasAnyFlags(RF_ClassDefaultObject | RF_NeedLoad))
	{
		return;
	}

#if WITH_EDITOR
	// Wait for the editor module to be set by the editor in UDialogueGraph constructor
	if (GetQuestEditorAccess().IsValid())
	{
		CreateGraph();
	}

	if (!IsValid(QuestGraph))
	{
		CreateGraph();
	}

#endif // #if WITH_EDITOR

	ID = GetQuestFName();

	if (!HasGUID())
	{
		RegenerateGUID();
	}
}

void UQuestAsset::PostRename(UObject* OldOuter, FName OldName)
{
	Super::PostRename(OldOuter, OldName);
	ID = GetQuestFName();
}

void UQuestAsset::PostDuplicate(bool bDuplicateForPIE)
{
	Super::PostDuplicate(bDuplicateForPIE);
	//When duplicating Quests a new GUID is needed!
	RegenerateGUID();
}

void UQuestAsset::PostEditImport()
{
	Super::PostEditImport();

	//When duplication a quest, a new GUID is needed
	RegenerateGUID();
}

#if WITH_EDITOR

bool UQuestAsset::Modify(bool bAlwaysMarkDirty)
{
	if (!CanModify())
	{
		return false;
	}

	const bool bWasSaved = Super::Modify(bAlwaysMarkDirty);

	return bWasSaved;
}

void UQuestAsset::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Signal to the listeners
	check(OnQuestPropertyChanged.IsBound());
	OnQuestPropertyChanged.Broadcast(PropertyChangedEvent);
}

void UQuestAsset::PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent)
{
	Super::PostEditChangeChainProperty(PropertyChangedEvent);
}

void UQuestAsset::AddReferencedObjects(UObject* InThis, FReferenceCollector& Collector)
{
	// Add the graph to the list of referenced objects
	UQuestAsset* This = CastChecked<UQuestAsset>(InThis);
	Collector.AddReferencedObject(This->QuestGraph, This);
	Super::AddReferencedObjects(InThis, Collector);
}


#endif // #if WITH_EDITOR


/* ***** END UObject ***** */

#if WITH_EDITOR
TSharedPtr<IQuestEditorAccess> UQuestAsset::QuestEditorAccess = nullptr;


/* ---- Editor Functions ---- */
void UQuestAsset::CreateGraph()
{
	//Graph is only null if it is created for the first time for this quest
	//After the quest asset is saved the Quest will get its graph from the serialized uasset
	if (QuestGraph != nullptr)
	{
		return;
	}

	//Startknoten erzeugen
	if (!IsValid(StartNode))
	{
		//StartNode = ConstructObjectiveNode<UQuestNode_Objective>();
		StartNode = ConstructQuestNode<UKillObjective>();
		if (Objectives.Num() > 0) {
			Objectives.Insert(StartNode, 0);
		}
		else
		{
			Objectives.Add(StartNode);
		}
	}
	//Graphen erzeugen
	QuestGraph = GetQuestEditorAccess()->CreateNewQuestGraph(this);

	//Give the schema a chance to fill out any required nodes
	QuestGraph->GetSchema()->CreateDefaultNodesForGraph(*QuestGraph);
	MarkPackageDirty();

}

void UQuestAsset::ClearGraph()
{
	if (!IsValid(QuestGraph))
	{
		return;
	}

	GetQuestEditorAccess()->RemoveAllGraphNodes(this);

	//Give the schema a chance to fill out any required nodes
	QuestGraph->GetSchema()->CreateDefaultNodesForGraph(*QuestGraph);
	MarkPackageDirty();
}

void UQuestAsset::CompileQuestNodesFromGraphNodes()
{
	if (!bCompileQuest)
	{
		return;
	}

	GetQuestEditorAccess()->CompileQuestNodesFromGraphNodes(this);
}

#endif // #if WITH_EDITOR

void UQuestAsset::SetNodes(const TArray<UObjective*>& InNodes)
{
	Objectives = InNodes;
	for (int32 NodeIndex = 0; NodeIndex < Objectives.Num(); NodeIndex++)
	{
		UpdateGUIDToIndexMap(Objectives[NodeIndex], NodeIndex);
	}
}

void UQuestAsset::SetStartNode(UObjective* InStartNode)
{
	if (!InStartNode)
	{
		return;
	}

	//Objectives need to be empty, only than a start objective should be created
	if (Objectives.Num() == 0) {
		Objectives.Add(InStartNode);
	}

	StartNode = InStartNode;

}

void UQuestAsset::UpdateGUIDToIndexMap(const UObjective* Node, int32 NodeIndex)
{
	if (!Node || !IsValidNodeIndex(NodeIndex) || !Node->HasGUID())
	{
		return;
	}

	NodesGUIDToIndexMap.Add(Node->GetGUID(), NodeIndex);
}




