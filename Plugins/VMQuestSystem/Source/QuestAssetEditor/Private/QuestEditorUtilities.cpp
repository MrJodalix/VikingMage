//TODO: Copyright
#include "QuestEditorUtilities.h"

#include "Toolkits/IToolkit.h"
#include "Toolkits/ToolkitManager.h"
#include "Templates/Casts.h"
#include "EdGraphNode_Comment.h"

#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/SClassPickerDialog.h"

#include "QuestAssetEditorToolkit.h"

/** Useful for auto positioning */
struct NodeWithParentPosition
{
	
	NodeWithParentPosition() {}
	NodeWithParentPosition(UQuestGraphNode* InNode, const int32 InParentNodeX, const int32 InParentNodeY) :
		Node(InNode), ParentNodeX(InParentNodeX), ParentNodeY(InParentNodeY) {}

	UQuestGraphNode* Node = nullptr;
	int32 ParentNodeX = 0;
	int32 ParentNodeY = 0;
};

/* ----- FQuestEditorUtilities ----- */

void FQuestEditorUtilities::LoadAllQuestsAndCheckGUIDs()
{
	
}

const TSet<UObject*> FQuestEditorUtilities::GetSelectedNodes(const UEdGraph* Graph)
{	
	TSharedPtr<FQuestAssetEditorToolkit> QuestEditor = GetQuestEditorForGraph(Graph);
	if (QuestEditor.IsValid())
	{
		return QuestEditor->GetSelectedNodes();
	}

	return {};
}

bool FQuestEditorUtilities::GetBoundsForSelectedNodes(const UEdGraph* Graph, FSlateRect& Rect, float Padding)
{
	//TODO (Testen): Nötig?
	TSharedPtr<FQuestAssetEditorToolkit> QuestEditor = GetQuestEditorForGraph(Graph);
	if (QuestEditor.IsValid())
	{
		return QuestEditor->GetBoundsForSelectedNodes(Rect, Padding);
	}

	return false;
	
}

void FQuestEditorUtilities::RefreshDetailsView(const UEdGraph* Graph, bool bRestorePreviousSelection)
{
	TSharedPtr<FQuestAssetEditorToolkit> DialogueEditor = GetQuestEditorForGraph(Graph);
	if (DialogueEditor.IsValid())
	{
		DialogueEditor->RefreshDetailsView(bRestorePreviousSelection);
	}
}

void FQuestEditorUtilities::Refresh(const UEdGraph* Graph, bool bRestorePreviousSelection)
{
	
	TSharedPtr<FQuestAssetEditorToolkit> DialogueEditor = GetQuestEditorForGraph(Graph);
	if (DialogueEditor.IsValid())
	{
		DialogueEditor->Refresh(bRestorePreviousSelection);
	}
}

void FQuestEditorUtilities::SimpleRefresh(const UEdGraph* Graph, bool bRestorePreviousSelection)
{

	TSharedPtr<FQuestAssetEditorToolkit> DialogueEditor = GetQuestEditorForGraph(Graph);
	if (DialogueEditor.IsValid())
	{
		DialogueEditor->RefreshViewport();
		DialogueEditor->RefreshDetailsView(bRestorePreviousSelection);
	}
}


bool FQuestEditorUtilities::RemoveNode(UEdGraphNode* NodeToRemove)
{
	if (!IsValid(NodeToRemove))
	{
		return false;
	}

	UQuestGraph* Graph = CastChecked<UQuestGraph>(NodeToRemove->GetGraph());
	if (!IsValid(Graph))
	{
		return false;
	}

	// Transactions should be declared in the code that calls this method
	if (!Graph->Modify())
	{		
		UE_LOG(LogTemp, Fatal, TEXT("FQuestEditorUtilities::RemoveNode No transaction was declared before calling this method, aborting!"));
		return false;
	}
	if (!NodeToRemove->Modify())
	{
		UE_LOG(LogTemp, Fatal, TEXT("FQuestEditorUtilities::RemoveNode No transaction was declared before calling this method, aborting!"));
		return false;
	}

	return Graph->RemoveGraphNode(NodeToRemove);
}



UEdGraph* FQuestEditorUtilities::CreateNewGraph(UObject* ParentScope, FName GraphName,
	TSubclassOf<UEdGraph> GraphClass, TSubclassOf<UEdGraphSchema> SchemaClass)
{
	// Mostly copied from FBlueprintEditorUtils::CreateNewGraph
	UEdGraph* NewGraph;
	bool bRename = false;

	// Ensure this name isn't already being used for a graph
	if (GraphName != NAME_None)
	{
		UEdGraph* ExistingGraph = FindObject<UEdGraph>(ParentScope, *(GraphName.ToString()));
		ensureMsgf(!ExistingGraph, TEXT("Graph %s already exists: %s"), *GraphName.ToString(), *ExistingGraph->GetFullName());

		// Rename the old graph out of the way; but we have already failed at this point
		if (ExistingGraph)
		{
			ExistingGraph->Rename(nullptr, ExistingGraph->GetOuter(), REN_DoNotDirty | REN_ForceNoResetLoaders);
		}

		// Construct new graph with the supplied name
		NewGraph = NewObject<UEdGraph>(ParentScope, GraphClass, NAME_None, RF_Transactional);
		bRename = true;
	}
	else
	{
		// Construct a new graph with a default name
		NewGraph = NewObject<UEdGraph>(ParentScope, GraphClass, NAME_None, RF_Transactional);
	}

	NewGraph->Schema = SchemaClass;

	// Now move to where we want it to. Workaround to ensure transaction buffer is correctly utilized
	if (bRename)
	{
		NewGraph->Rename(*GraphName.ToString(), ParentScope, REN_DoNotDirty | REN_ForceNoResetLoaders);
	}

	return NewGraph;
	
}


bool FQuestEditorUtilities::AreQuestNodesInSyncWithGraphNodes(const UQuestAsset* Quest)
{		
	const int32 NumGraphNodes = CastChecked<UQuestGraph>(Quest->GetGraph())->GetAllQuestGraphNodes().Num();
	const int32 NumDialogueNodes = Quest->GetObjectives().Num(); 
	if (NumGraphNodes == NumDialogueNodes)
	{
		return true;
	}

	return false;
}

UObjective* FQuestEditorUtilities::GetClosestNodeFromGraphNode(UEdGraphNode* GraphNode)
{
	const UQuestGraphNode_Base* BaseNode = Cast<UQuestGraphNode_Base>(GraphNode);
	if (!BaseNode)
	{
		return nullptr;
	}

	// Node
	if (const UQuestGraphNode* Node = Cast<UQuestGraphNode>(BaseNode))
	{
		return Node->GetMutableQuestNode();
	}

	return nullptr;
}

void FQuestEditorUtilities::TryToCreateDefaultGraph(UQuestAsset* Quest, bool bPrompt)
{
	// Clear the graph if the number of nodes differ	
	if(AreQuestNodesInSyncWithGraphNodes(Quest))
	{
		return;
	}

	// Simply do the operations without any consent
	//if (!bPrompt)
	//{		
		//CheckAndTryToFixDialogue(Dialogue);
		Quest->ClearGraph();
		return;
	//}
}


TSharedPtr<FQuestAssetEditorToolkit> FQuestEditorUtilities::GetQuestEditorForGraph(const UEdGraph* Graph)
{
	//Find the associated Quest
	const UQuestAsset* Quest = GetQuestForGraph(Graph);
	TSharedPtr<FQuestAssetEditorToolkit> QuestEditor;

	//This Quest has already an asset editor opened
	TSharedPtr<IToolkit> FoundAssetEditor = FToolkitManager::Get().FindEditorForAsset(Quest);
	if(FoundAssetEditor.IsValid())
	{
		QuestEditor = StaticCastSharedPtr<FQuestAssetEditorToolkit>(FoundAssetEditor);
	}

	return QuestEditor;
}



