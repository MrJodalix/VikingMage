//TODO: Copyright
#include "QuestGraphSchema.h"

#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "UObject/UObjectIterator.h"
#include "ScopedTransaction.h"
#include "AssetData.h"
#include "GraphEditorActions.h"

#include "ToolMenu.h"
#include "ToolMenuSection.h"

#include "QuestGraph.h"
#include "QuestEditorUtilities.h"
#include "Toolkits/Nodes/QuestGraphNode.h"
#include "SchemaActions/NewNode_QuestGraphSchemaAction.h"
#include "SchemaActions/NewComment_QuestGraphSchemaAction.h"

#define LOCTEXT_NAMESPACE "QuestGraphSchema"

// Initialize static properties
const FName UQuestGraphSchema::PIN_CATEGORY_Input(TEXT("ParentInputs"));
const FName UQuestGraphSchema::PIN_CATEGORY_Output(TEXT("ChildOutputs"));

const FText UQuestGraphSchema::NODE_CATEGORY_Quest(LOCTEXT("NewObejctiveAction", "New Objectives"));
const FText UQuestGraphSchema::NODE_CATEGORY_Graph(LOCTEXT("GraphAction", "Graph"));

TArray<TSubclassOf<UObjective>> UQuestGraphSchema::QuestNodeClasses;
bool UQuestGraphSchema::bQuestNodeClassesInitialized = false;

//////////////////////////////////////////////////////////
// UQuestGraphSchema
void UQuestGraphSchema::GetPaletteActions(FGraphActionMenuBuilder& ActionMenuBuilder) const
{
	GetAllQuestNodeActions(ActionMenuBuilder);
	GetCommentAction(ActionMenuBuilder);
}

bool UQuestGraphSchema::ConnectionCausesLoop(const UEdGraphPin* InputPin, const UEdGraphPin* OutputPin) const
{
	UQuestGraphNode_Base* InputNode = CastChecked<UQuestGraphNode_Base>(InputPin->GetOwningNode());
	UQuestGraphNode_Base* OutputNode = CastChecked<UQuestGraphNode_Base>(OutputPin->GetOwningNode());

	//Same Node
	if(InputNode == OutputNode)
	{
		return true;
	}
	return false;	
}

/* ----- EdGraphSchema Interface ----- */
void UQuestGraphSchema::GetGraphContextActions(FGraphContextMenuBuilder& ContextMenuBuilder) const
{
	GetAllQuestNodeActions(ContextMenuBuilder);
	GetCommentAction(ContextMenuBuilder, ContextMenuBuilder.CurrentGraph);

	//Menu not from a pin, directly right clicked on the graph canvas
	if(!ContextMenuBuilder.FromPin)
	{
		//TODO paste nodes here?!
	}
}

void UQuestGraphSchema::GetContextMenuActions(UToolMenu* Menu, UGraphNodeContextMenuContext* Context) const
{
	if(Context->Node && !Context->bIsDebugging)
	{				
		// Menu for right clicking on node
		FToolMenuSection& Section = Menu->AddSection("QuestGraphSchemaNodeActions", LOCTEXT("NodeActionsMenuHeader", "Node Actions"));

		// This action is handled in UQuestGraphSchema::BreakNodeLinks, and the action is registered in SGraphEditorImpl (not by us)
		Section.AddMenuEntry(FGraphEditorCommands::Get().BreakNodeLinks);	
	}
	// The rest of the menus are implemented in the each nodes GetContextMenuActions method
	Super::GetContextMenuActions(Menu, Context);	
}

void UQuestGraphSchema::CreateDefaultNodesForGraph(UEdGraph& Graph) const
{
	// This should only be called on empty graphs
	check(Graph.Nodes.Num() == 0);
	UQuestGraph* QuestGraph = CastChecked<UQuestGraph>(&Graph);

	// Create, link and position nodes
	QuestGraph->CreateGraphNodesFromQuest();
	QuestGraph->LinkGraphNodesFromQuest();
	QuestGraph->AutoPositionGraphNodes();

	// TODO(vampy): Fix editor crash
	//SetNodeMetaData(ResultRootNode, FNodeMetadata::DefaultGraphNode);
}

FPinConnectionResponse UQuestGraphSchema::MovePinLinks(UEdGraphPin& MoveFromPin, UEdGraphPin& MoveToPin, bool bIsIntermediateMove, bool bNotifyLinkedNodes) const
{
	return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, LOCTEXT("ConnectionMovePinLinks", "Move Pin Links Not implemented"));
}

FPinConnectionResponse UQuestGraphSchema::CopyPinLinks(UEdGraphPin& CopyFromPin, UEdGraphPin& CopyToPin, bool bIsIntermediateCopy) const
{
	return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, LOCTEXT("ConnectionMovePinLinks", "Copy Pin Links Not implemented"));
}

const FPinConnectionResponse UQuestGraphSchema::CanCreateConnection(const UEdGraphPin* PinA, const UEdGraphPin* PinB) const
{
	// Make sure the pins are not on the same node
	if (PinA->GetOwningNode() == PinB->GetOwningNode())
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, LOCTEXT("ConnectionSameNode", "Both are on the same node"));
	}

	// Causes loop
	if (ConnectionCausesLoop(PinA, PinB))
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, LOCTEXT("ConnectionLoop", "Connection would cause loop"));
	}

	// Compare the directions
	const UQuestGraphNode_Base* SourceNode = CastChecked<UQuestGraphNode_Base>(PinA->GetOwningNode());
	const UQuestGraphNode_Base* TargetNode = CastChecked<UQuestGraphNode_Base>(PinB->GetOwningNode());

	// Does the source Node accept output connection?
	if (!SourceNode->CanHaveOutputConnections())
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, TEXT("Cannot wire an edge from this node because it does not accept output connection "));
	}

	// Does the targe Node accept input connection?
	if (!TargetNode->CanHaveInputConnections())
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, TEXT("Cannot wire an edge to this node because it does not accept input connection "));
	}

	// Only allow one connection from an output (parent node)
	if (SourceNode->HasOutputConnectionToNode(TargetNode))
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, LOCTEXT("ConnectionAlreadyMade", "Connection between nodes already made"));
	}
	
	return FPinConnectionResponse(CONNECT_RESPONSE_MAKE, FText::GetEmpty());	
}

bool UQuestGraphSchema::TryCreateConnection(UEdGraphPin* PinA, UEdGraphPin* PinB) const
{
	// Happens when connecting pin to itself, seems to be a editor bug
	if (PinA->GetOwningNode() == PinB->GetOwningNode())
	{
		return false;
	}
	// Handle the CONNECT_RESPONSE_BREAK_OTHERS, this should handle all cases, because there is only
	// one input pin and that is the only one which will loose it's former parent
	UEdGraphPin* FormerParentOutputPin = nullptr;

	// Mark for undo system, we do not know if there is transaction so just mark without verifying
	// This mostly fixes crashing on undo when there is a drag operation
	UEdGraph* Graph = PinA->GetOwningNode()->GetGraph();
	{
		PinA->GetOwningNode()->Modify();
		PinB->GetOwningNode()->Modify();
		Graph->Modify();
		FQuestEditorUtilities::GetQuestForGraph(Graph)->Modify();
	}

	const bool bModified = Super::TryCreateConnection(PinA, PinB);
	
	if (bModified)
	{
		// Notify former parent
		if (FormerParentOutputPin != nullptr)
		{
			FormerParentOutputPin->GetOwningNode()->PinConnectionListChanged(FormerParentOutputPin);
		}

		UQuestGraphNode_Base* NodeB = CastChecked<UQuestGraphNode_Base>(PinB->GetOwningNode());

		UQuestAsset* Quest = FQuestEditorUtilities::GetQuestForGraph(Graph);
		// Update the internal structure (recompile of the Dialogue Node/Graph Nodes)
		Quest->CompileQuestNodesFromGraphNodes();
		
		FQuestEditorUtilities::SimpleRefresh(Quest->GetGraph(), true);
		UE_LOG(LogTemp, Display, TEXT("Quest Editor: Created new Connection. Recompiled."));
	}

	// Reset the value
	// //TODO: Was hier?
	//FQuestEditorUtilities::SetLastTargetGraphEdgeBeforeDrag(Graph, nullptr);

	return bModified;
	
}

bool UQuestGraphSchema::CreateAutomaticConversionNodeAndConnections(UEdGraphPin* PinA, UEdGraphPin* PinB) const
{
	//TODO?
	return Super::CreateAutomaticConversionNodeAndConnections(PinA, PinB);	
}

bool UQuestGraphSchema::ShouldHidePinDefaultValue(UEdGraphPin* Pin) const
{
	return true;
}

void UQuestGraphSchema::BreakNodeLinks(UEdGraphNode& TargetNode) const
{	
	// NOTE: The SGraphEditorImpl::BreakNodeLinks that calls this (method) has the transaction declared, so do not make another one here.
	UEdGraph* Graph = TargetNode.GetGraph();
	UQuestAsset* Quest = FQuestEditorUtilities::GetQuestForGraph(Graph);

	// Mark for undo system
	verify(Graph->Modify());
	verify(TargetNode.Modify());
	verify(Quest->Modify());

	Super::BreakNodeLinks(TargetNode);

#if DO_CHECK
	if (UQuestGraphNode* GraphNode = Cast<UQuestGraphNode>(&TargetNode))
	{
		GraphNode->CheckAll();
	}
#endif

	Quest->CompileQuestNodesFromGraphNodes();
	FQuestEditorUtilities::Refresh(Quest->GetGraph(), true);
	UE_LOG(LogTemp, Display, TEXT("Quest Editor: Broke Node Links. Recompiled."));
}

void UQuestGraphSchema::BreakPinLinks(UEdGraphPin& TargetPin, bool bSendsNodeNotifcation) const
{
	
	const FScopedTransaction Transaction(LOCTEXT("GraphEd_BreakPinLinks", "Quest Editor: Break Pin Links"));
	UEdGraphNode* Node = TargetPin.GetOwningNode();
	UEdGraph* Graph = Node->GetGraph();
	UQuestAsset* Quest = FQuestEditorUtilities::GetQuestForGraph(Graph);

	// Mark for undo system
	verify(Node->Modify());
	verify(Graph->Modify());
	verify(Quest->Modify());
	// Modify() is called in BreakLinkTo on the TargetPin

	Super::BreakPinLinks(TargetPin, bSendsNodeNotifcation);

#if DO_CHECK
	if (UQuestGraphNode* GraphNode = Cast<UQuestGraphNode>(TargetPin.GetOwningNode()))
	{
		GraphNode->CheckAll();
	}
#endif

	// If this would notify the node then we need to compile the Dialogue
	if (bSendsNodeNotifcation)
	{
		// Recompile
		Quest->CompileQuestNodesFromGraphNodes();
		FQuestEditorUtilities::Refresh(Quest->GetGraph(), true);
		UE_LOG(LogTemp, Display, TEXT("Quest Editor: Broke Pin Links. Recompiled."));
	}
}

void UQuestGraphSchema::BreakSinglePinLink(UEdGraphPin* SourcePin, UEdGraphPin* TargetPin) const
{
	const FScopedTransaction Transaction(LOCTEXT("GraphEd_BreakSinglePinLink", "Quest Editor: Break Pin Link"));
	// Modify() is called in BreakLinkTo
	Super::BreakSinglePinLink(SourcePin, TargetPin);
}

void UQuestGraphSchema::DroppedAssetsOnGraph(const TArray<FAssetData>& Assets, const FVector2D& GraphPosition, UEdGraph* Graph) const
{
	
}

void UQuestGraphSchema::DroppedAssetsOnNode(const TArray<FAssetData>& Assets, const FVector2D& GraphPosition, UEdGraphNode* Node) const
{
	
}

/* ***** EdGraphSchema Interface ***** */

/* ----- Own Functions ----- */
void UQuestGraphSchema::BreakLinkTo(UEdGraphPin* FromPin, UEdGraphPin* ToPin, bool bSendsNodeNotifcation) const
{
	const FScopedTransaction Transaction(LOCTEXT("GraphEd_BreakPinLink", "Dialogue Editor: Break Pin Link"));
	UEdGraphNode* FromNode = FromPin->GetOwningNode();
	UEdGraphNode* ToNode = ToPin->GetOwningNode();
	UEdGraph* Graph = FromNode->GetGraph();
	UQuestAsset* Quest = FQuestEditorUtilities::GetQuestForGraph(Graph);

	// Mark for undo system
	verify(FromNode->Modify());
	verify(ToNode->Modify());
	verify(Graph->Modify());
	verify(Quest->Modify());

	// Break
	FromPin->BreakLinkTo(ToPin);

	// Notify
	FromNode->PinConnectionListChanged(FromPin);
	ToNode->PinConnectionListChanged(ToPin);
	if (bSendsNodeNotifcation)
	{
		FromNode->NodeConnectionListChanged();
		ToNode->NodeConnectionListChanged();
	}

#if DO_CHECK
	if (UQuestGraphNode* GraphNode = Cast<UQuestGraphNode>(FromNode))
	{
		GraphNode->CheckAll();
	}
	if (UQuestGraphNode* GraphNode = Cast<UQuestGraphNode>(ToNode))
	{
		GraphNode->CheckAll();
	}
#endif

	// If this would notify the node then we need to Recompile the Dialogue
	if (bSendsNodeNotifcation)
	{
		Quest->CompileQuestNodesFromGraphNodes();
	}
}

void UQuestGraphSchema::GetCommentAction(FGraphActionMenuBuilder& ActionMenuBuilder, const UEdGraph* CurrentGraph) const
{
	// Do not allow to spawn a comment when we drag are dragging from a selected pin.
	if (ActionMenuBuilder.FromPin)
	{
		return;
	}

	// The rest of the comment actions are in the UEdGraphSchema::GetContextMenuActions
	const bool bIsManyNodesSelected = CurrentGraph ? GetNodeSelectionCount(CurrentGraph) > 0 : false;
	const FText MenuDescription = bIsManyNodesSelected ?
		LOCTEXT("CreateCommentAction", "Create Comment from Selection") : LOCTEXT("AddCommentAction", "Add Comment...");
	const FText ToolTip = LOCTEXT("CreateCommentToolTip", "Creates a comment.");
	constexpr int32 Grouping = 0;

	TSharedPtr<FNewComment_QuestGraphSchemaAction> NewAction(new FNewComment_QuestGraphSchemaAction(
		NODE_CATEGORY_Graph, MenuDescription, ToolTip, Grouping));
	ActionMenuBuilder.AddAction(NewAction);
}

void UQuestGraphSchema::GetAllQuestNodeActions(FGraphActionMenuBuilder& ActionMenuBuilder) const
{
	InitQuestNodeClasses();
	FText ToolTip, MenuDesc;

	// when dragging from an input pin
	if (ActionMenuBuilder.FromPin == nullptr)
	{
		// Just right clicked on the empty graph
		ToolTip = LOCTEXT("NewQuestNodeTooltip", "Adds {Name} to the graph");
		MenuDesc = LOCTEXT("NewQuestNodeMenuDescription", "{Name}");
	}
	else if (ActionMenuBuilder.FromPin->Direction == EGPD_Input)
	{
		// From an input pin
		ToolTip = LOCTEXT("NewQuestNodeTooltip_FromInputPin", "Adds {Name} to the graph as a parent to the current node");
		MenuDesc = LOCTEXT("NewQuestNodeMenuDescription_FromInputPin", "Add {Name} parent");
	}
	else
	{
		// From an output pin
		check(ActionMenuBuilder.FromPin->Direction == EGPD_Output);
		ToolTip = LOCTEXT("NewQuestNodeTooltip_FromOutputPin", "Adds {Name} to the graph as a child to the current node");
		MenuDesc = LOCTEXT("NewQuestNodeMenuDescription_FromOutputPin", "Add {Name} child");
	}

	int32 Grouping = 1;
	FFormatNamedArguments Arguments;

	// Generate menu actions for all the node types
	for (TSubclassOf<UObjective> QuestNodeClass : QuestNodeClasses)
	{
		const UObjective* QuestNode = QuestNodeClass->GetDefaultObject<UObjective>();
		Arguments.Add(TEXT("Name"), FText::FromString(QuestNode->GetNodeTypeString() + " Objective"));

		TSharedPtr<FNewNode_QuestGraphSchemaAction> Action(new FNewNode_QuestGraphSchemaAction(
			NODE_CATEGORY_Quest, FText::Format(MenuDesc, Arguments), FText::Format(ToolTip, Arguments),
			Grouping, QuestNodeClass));
		ActionMenuBuilder.AddAction(Action);
	}
	
}

void UQuestGraphSchema::InitQuestNodeClasses()
{
	//Early exit, if already initialized
	if(bQuestNodeClassesInitialized)
	{
		return;
	}

	// Construct list of non-abstract quest node classes (objectives).
	for (TObjectIterator<UClass> It; It; ++It)
	{
		if (It->IsChildOf(UObjective::StaticClass()) && !It->HasAnyClassFlags(CLASS_Abstract))
		{
			QuestNodeClasses.Add(*It);
		}
	}
	
	bQuestNodeClassesInitialized = true;
}

/* ***** END own functions ***** */


#undef LOCTEXT_NAMESPACE