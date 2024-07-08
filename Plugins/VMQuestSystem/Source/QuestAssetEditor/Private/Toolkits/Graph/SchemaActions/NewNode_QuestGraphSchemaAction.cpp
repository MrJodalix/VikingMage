//TODO: Copyright
#include "NewNode_QuestGraphSchemaAction.h"

#include "ScopedTransaction.h"

#include "QuestAsset.h"
#include "Toolkits/Nodes/QuestGraphNode.h"

#define LOCTEXT_NAMESPACE "NewNode_QuestGraphSchemaAction"

UEdGraphNode* FNewNode_QuestGraphSchemaAction::PerformAction(
	UEdGraph* ParentGraph, 
	UEdGraphPin* FromPin, 
	const FVector2D Location, 
	bool bSelectNewNode)
{
	const FScopedTransaction Transaction(LOCTEXT("QuesteditorNewDialogueNode", "Quest Editor: New Quest Node"));
	UQuestAsset* Quest = CastChecked<UQuestGraph>(ParentGraph)->GetQuestAsset();

	//Mark for modification
	verify(ParentGraph->Modify());
	if(FromPin)
	{
		verify(FromPin->Modify());
	}
	verify(Quest->Modify());

	//Create node, without needing to compile it
	UEdGraphNode* GraphNode = CreateNode(Quest, ParentGraph, FromPin, Location, bSelectNewNode);
	Quest->PostEditChange();
	Quest->MarkPackageDirty();
	ParentGraph->NotifyGraphChanged();

	return GraphNode;
}

UEdGraphNode* FNewNode_QuestGraphSchemaAction::CreateNode(
	UQuestAsset* QuestAsset, 
	UEdGraph* ParentGraph, 
	UEdGraphPin* FromPin, 
	FVector2D Location, 
	bool bSelectNewNode)
{
	// Maximum distance a drag can be off a node edge to require 'push off' from node
	static constexpr int32 NodeDistance = 60;

	//Create the quest node
	auto QuestNode = QuestAsset->ConstructQuestNode<UObjective>(CreateNodeType);

	//Create the graph node
	FGraphNodeCreator<UQuestGraphNode> NodeCreator(*ParentGraph);
	UQuestGraphNode* GraphNode = NodeCreator.CreateUserInvokedNode(bSelectNewNode);

	//Link quest node <-> graph node
	QuestNode->SetGraphNode(GraphNode);
	const int32 QuestNodeIndex = QuestAsset->AddNode(QuestNode);
	GraphNode->SetQuestNodeDataChecked(QuestNodeIndex, QuestNode);

	//Finalize graph node creation
	NodeCreator.Finalize(); // Calls on the node: CreateNewGuid, PostPlacedNewNode, AllocateDefaultPins
	GraphNode->AutowireNewNode(FromPin);

	//Position graph node
	// For input pins, new node will generally overlap node being dragged off
	// Work out if we want to visually push away from connected node
	int32 XLocation = Location.X;
	if(FromPin && FromPin->Direction == EGPD_Input)
	{
		UEdGraphNode* PinNode = FromPin->GetOwningNode();
		const float XDelta = FMath::Abs(PinNode->NodePosX - Location.X);

		if(XDelta < NodeDistance)
		{
			// Set location to edge of current node minus the max move distance
			// to force node to push off from connect node enough to give selection handle
			XLocation = PinNode->NodePosX - NodeDistance;
		}
	}

	GraphNode->SetPosition(XLocation, Location.Y);
	return CastChecked<UEdGraphNode>(GraphNode);	
}

#undef LOCTEXT_NAMESPACE

