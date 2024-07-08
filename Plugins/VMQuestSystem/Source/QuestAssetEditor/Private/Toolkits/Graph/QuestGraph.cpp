//TODO: Copyright
#include "QuestGraph.h"

#include "GraphEditAction.h"

#include "QuestAsset/Private/KillObjective.h"
#include "QuestAsset.h"
#include "QuestEditorAccess.h"
#include "QuestGraphSchema.h"
#include "Toolkits/Nodes/QuestGraphNode_Root.h"
#include "Toolkits/Nodes/QuestGraphNode.h"
#include "Toolkits/Nodes/QuestGraphNode_Edge.h"

UQuestGraph::UQuestGraph(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Set the static editor module interface used by all the quests in the QuestSystem module to communicate with the editor.
	if (!UQuestAsset::GetQuestEditorAccess().IsValid())
	{
		UQuestAsset::SetQuestEditorAccess(TSharedPtr<IQuestEditorAccess>(new FQuestEditorAccess));
	}
}

bool UQuestGraph::Modify(bool bAlwaysMarkDirty)
{
	if (!CanModify())
	{
		return false;
	}

	bool bWasSaved = Super::Modify(bAlwaysMarkDirty);
	// Transactions do not support arrays?
	// See https://answers.unrealengine.com/questions/674286/how-to-undoredo-a-modification-to-an-array.html?sort=oldest
	// TODO check out why it does not save the arrays, because it uses the standard serializer that also writes to .uasset

	// Mark all nodes for modification
	// question of space (save them all here) or recompile them after every undo
	for (UQuestGraphNode_Base* BaseNode : GetAllBaseQuestGraphNodes())
	{
		bWasSaved = bWasSaved && BaseNode->Modify(bAlwaysMarkDirty);
	}

	return bWasSaved;
}

UQuestGraphNode* UQuestGraph::GetRootGraphNode() 
{
	//Fix RootNode!
	if(!RootNode)
	{
		for(UEdGraphNode* Node : Nodes)
		{
			UQuestGraphNode* CurrentNode = Cast<UQuestGraphNode>(Node);
			if(CurrentNode && CurrentNode->IsRootNode())
			{
				RootNode = CurrentNode;
				break;				
			}
		}		
	}
	check(RootNode);	
	return RootNode;
}

TArray<UQuestGraphNode_Base*> UQuestGraph::GetAllBaseQuestGraphNodes() const
{
	TArray<UQuestGraphNode_Base*> AllBaseQuestGraphnodes;
	GetNodesOfClass<UQuestGraphNode_Base>(/*out*/ AllBaseQuestGraphnodes);
	return AllBaseQuestGraphnodes;
}

TArray<UQuestGraphNode*> UQuestGraph::GetAllQuestGraphNodes() const
{
	TArray<UQuestGraphNode*> AllQuestGraphNodes;
	GetNodesOfClass<UQuestGraphNode>(/*out*/ AllQuestGraphNodes);
	return AllQuestGraphNodes;
}

bool UQuestGraph::RemoveGraphNode(UEdGraphNode* NodeToRemove)
{
	Modify();
	const int32 NumTimesNodeRemoved = Nodes.Remove(NodeToRemove);

	// This will trigger the compile in the UDialogueGraphSchema::BreakNodeLinks
	// NOTE: do not call BreakAllNodeLinks on the node as it does not register properly with the
	// undo system
	GetSchema()->BreakNodeLinks(*NodeToRemove);

	// Notify
	FEdGraphEditAction RemovalAction;
	RemovalAction.Graph = this;
	RemovalAction.Action = GRAPHACTION_RemoveNode;
	RemovalAction.Nodes.Add(NodeToRemove);
	NotifyGraphChanged(RemovalAction);

	return NumTimesNodeRemoved > 0;
}

void UQuestGraph::CreateGraphNodesFromQuest()
{
	//TODO: Debug
	UE_LOG(LogTemp, Warning, TEXT("Quest System: Quest Graph - Create Nodes From Quest (Start)"));
	
	// Assume empty graph
	check(Nodes.Num() == 0);
	UQuestAsset* Quest = GetQuestAsset();
	//TODO (Testen): Nötig?
	//FQuestEditorUtilities::CheckAndTryToFixDialogue(Dialogue, false);

	// Step 1: Create the root (start) node
	{
		FGraphNodeCreator<UQuestGraphNode> NodeCreator(*this);
		UQuestGraphNode* StartGraphNode = NodeCreator.CreateNode();
		
		// Create two way direction for both Dialogue Node and Graph Node.

		//check if objective exists
		TArray<UObjective*> Objectives = Quest->GetObjectives();
		UObjective* StartNode;
		if(Objectives.Num() > 0)
		{
			StartNode = Objectives[0];
		}
		else
		{
			//Fallback, normally first objective should be created with creation of the graph!
			StartNode = NewObject<UKillObjective>();			
		}	
		
		check(StartNode);

		StartGraphNode->SetQuestNode(StartNode);
		RootNode = StartGraphNode;
		//Will only be set, if quest is empty
		Quest->SetStartNode(StartNode);

		// Finalize creation
		StartGraphNode->SetPosition(0, 0);
		NodeCreator.Finalize();

		
		
	}
	// Step 2: Create the Graph Nodes for all the Nodes
	const TArray<UObjective*>& Objectives = Quest->GetObjectives();
	const int32 NodesNum = Objectives.Num();	
	for (int32 NodeIndex = 1; NodeIndex < NodesNum; NodeIndex++)
	{
		FGraphNodeCreator<UQuestGraphNode> NodeCreator(*this);
		UQuestGraphNode* GraphNode = NodeCreator.CreateNode();

		// Create two way direction for both Dialogue Node and Graph Node.
		GraphNode->SetQuestNodeDataChecked(NodeIndex, Objectives[NodeIndex]);

		// Finalize creation
		GraphNode->SetPosition(0, 0);
		NodeCreator.Finalize();
	}

	//TODO: Debug
	UE_LOG(LogTemp, Warning, TEXT("Quest Syste: Quest Graph - Create Nodes From Quest (END)"));
	
}

void UQuestGraph::LinkGraphNodesFromQuest()
{
	UQuestGraphNode* StartNodeGraph = GetRootGraphNode();
	UQuestAsset* Quest = GetQuestAsset();
		
	// Assume we have all the nodes created
	checkf(Quest->GetObjectives().Num() == GetAllQuestGraphNodes().Num(), TEXT("Objectives and GraphNodes should be equal but they are %d (Objectives) and %d (Nodes)..."), Quest->GetObjectives().Num(), GetAllQuestGraphNodes().Num());

	const UObjective& StartObjective = Quest->GetStartObjective();
	const TArray<UObjective*>& Objectives = Quest->GetObjectives();
	
	// Step 1. Make the root (start) node connections
	LinkGraphNodeToChildren(Objectives, StartObjective, StartNodeGraph);

	// Step 2: Create all the connections between the rest of the nodes
	for (UObjective* ObjectiveNode : Objectives)
	{
		LinkGraphNodeToChildren(Objectives, *ObjectiveNode, CastChecked<UQuestGraphNode>(ObjectiveNode->GetGraphNode()));
	}
}

void UQuestGraph::LinkGraphNodeToChildren(const TArray<UObjective*>& NodesObjective, const UObjective& NodeObjective, UQuestGraphNode* NodeGraph) const
{
	// Assume we are starting from scratch, no output connections
	NodeGraph->GetOutputPin()->BreakAllPinLinks();

	const TArray<UObjective*>& ChildObjectives = NodeObjective.GetNodeChildren();
	TSet<UObjective*> NodeSeenChildren;
		
	//Loop through all children on the node
	for (int32 ChildIndex = 0, ChildNum = ChildObjectives.Num(); ChildIndex < ChildNum; ChildIndex++)
	{
		// Prevent two edges to the same node.
		if (NodeSeenChildren.Contains(ChildObjectives[ChildIndex]))
		{
			continue;
		}

		// Get the child node and make sure it has the required number of inputs
		const UObjective& ChildNode = *ChildObjectives[ChildIndex];
		UQuestGraphNode* ChildGraphNode = CastChecked<UQuestGraphNode>(ChildNode.GetGraphNode());

		//Make connection from parent to child
		UEdGraphPin* ParentPin = NodeGraph->GetOutputPin();
		UEdGraphPin* ChildPin = ChildGraphNode->GetInputPin();

		ParentPin->MakeLinkTo(ChildPin);
		
		//Remember child as already visited
		NodeSeenChildren.Add(ChildObjectives[ChildIndex]);
	}
}

void UQuestGraph::AutoPositionGraphNodes()
{
	//TODO: Bei bedarf implementieren
	UE_LOG(LogTemp, Warning, TEXT("UQuestGraph::AutoPositionGraphNodes() not implemented."));		
	static constexpr bool bIsDirectionVertical = true;
	UQuestGraphNode* StartNode = GetRootGraphNode();
	const TArray<UQuestGraphNode*> QuestGraphNodes = GetAllQuestGraphNodes();
	
	/*
	FQuestEditorUtilities::AutoPositionGraphNodes(
		RootNode,
		QuestGraphNodes,
		500,
		200,
		bIsDirectionVertical
	);*/
}

void UQuestGraph::RemoveAllNodes()
{
	Modify();

	// Could have used RemoveNode on each node but that is unnecessary as that is slow and notifies external objects
	Nodes.Empty();
	check(Nodes.Num() == 0);
}

const UQuestGraphSchema* UQuestGraph::GetQuestGraphSchema() const
{
	return GetDefault<UQuestGraphSchema>(Schema);
}






