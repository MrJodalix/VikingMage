//TODO: Copyright
#include "QuestGraphNode.h"

#include "Editor/EditorEngine.h"
#include "Framework/Commands/GenericCommands.h"
#include "EdGraph/EdGraphNode.h"
#include "Engine/Font.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Runtime/Launch/Resources/Version.h"

#include "ToolMenu.h"

#include "QuestAsset/Private/KillObjective.h"
#include "QuestAsset/Private/CollectObjective.h"
#include "QuestAsset/Private/LocationObjective.h"
#include "QuestAsset/Private/InteractObjective.h"
#include "QuestAsset.h"
#include "QuestAssetEditorToolkit.h"
#include "QuestCommands.h"
#include "QuestEditorUtilities.h"

#define LOCTEXT_NAMESPACE "QuestGraphNode"

/* ---- UObject interface ----- */

void UQuestGraphNode::PostLoad()
{
	Super::PostLoad();

	//Fix any QuestNode pointers that may be out of date
	if (QuestNode)
	{
		QuestNode->SetGraphNode(this);
		QuestNode->SetFlags(RF_Transactional);
	}
}

void UQuestGraphNode::PostEditImport()
{
	RegisterListeners();

	// Make sure this QuestNode is owned by the Quest it's being pasted into.
	// The paste can come from another Quest
	ResetQuestNodeOwner();
}

void UQuestGraphNode::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (!PropertyChangedEvent.Property)
	{
		return;
	}

	CheckAll();
	ApplyCompilerWarnings();
}

void UQuestGraphNode::PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent)
{
	Super::PostEditChangeChainProperty(PropertyChangedEvent);
}

bool UQuestGraphNode::Modify(bool bAlwaysMarkDirty)
{
	if (!CanModify())
	{
		return false;
	}

	bool bWasModified = Super::Modify(bAlwaysMarkDirty);

	// Notify the Quest structure of modification
	if (QuestNode)
	{
		bWasModified = bWasModified && QuestNode->Modify(bAlwaysMarkDirty);
	}

	// Special case when this method is called when the engine is starting
	/*if (HasOutputPin())
	{
		// Can happen when copy pasting nodes
		constexpr bool bCheckParent = false;
		for (UDialogueGraphNode_Edge* EdgeNode : GetChildEdgeNodes(bCheckParent))
		{
			bWasModified = bWasModified && EdgeNode->SuperModify();
		}
	}*/

	return bWasModified;
}

/* ***** END UObject interface ***** */


/* ----- Begin UEdGraphNode interface ----- */

FText UQuestGraphNode::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	if (NodeIndex == INDEX_NONE)
	{
		return Super::GetNodeTitle(TitleType);
	}

	FString FullString = IsRootNode() ? FString("Root: ") : FString("");
	FullString.Append(QuestNode->GetObjectiveTitle().ToString().IsEmpty() ?
		QuestNode->GetObjectiveType().ToString() : QuestNode->GetObjectiveTitle().ToString());
			
	// Display the full title
	return FText::FromString(FullString);
}

void UQuestGraphNode::PrepareForCopying()
{
	Super::PrepareForCopying();

	// Temporarily take ownership of the DialogueNode, so that it is not deleted when cutting
	if (QuestNode)
	{
		QuestNode->Rename(nullptr, this, REN_DontCreateRedirectors);
	}
}

void UQuestGraphNode::PostCopyNode()
{
	Super::PostCopyNode();

	// Make sure the Quest goes back to being owned by the Quest after copying.
	ResetQuestNodeOwner();
}

FString UQuestGraphNode::GetDocumentationExcerptName() const
{
	return "";
}

FText UQuestGraphNode::GetTooltipText() const
{
	//TODO: Mehr Tooltips?

	FFormatNamedArguments Args;
	Args.Add(TEXT("ObjectiveType"), QuestNode ? QuestNode->GetObjectiveType() : FText::GetEmpty());

	return FText::Format(LOCTEXT("QuestGraphNodeTooltip", "{ObjectiveType}"), Args);
}

void UQuestGraphNode::PinConnectionListChanged(UEdGraphPin* Pin)
{

	check(Pin->GetOwningNode() == this);
	//TODO: Edges?
	//check(QuestNode->GetNodeOpenChildren_DEPRECATED().Num() == 0);

	// Input pins are ignored, as they are not reliable source of information, each node should only work with its output pins
	if (Pin->Direction == EGPD_Input)
	{
		return;
	}

	// Handle Output Pin (this node)
	check(Pin->Direction == EGPD_Output);
	const UEdGraphPin* OutputPin = GetOutputPin();
	// Only one Pin, and that pin should be our output pinOutputPin
	check(Pin == OutputPin);

	//TODO: Edges?
	/*
	//const int32 DialogueNodeChildrenNum = QuestNode->GetNodeChildren().Num();
	const int32 GraphNodeChildrenNum = OutputPin->LinkedTo.Num();

	// Nothing added/removed, maybe something replaced?
	if (DialogueNodeChildrenNum == GraphNodeChildrenNum)
	{
#if DO_CHECK
		const FDiffNodeEdgeLinkedToPinResult& DiffResult = FindDifferenceBetweenNodeEdgesAndLinkedToPins();
		check(DiffResult.Index == INDEX_NONE);
		check(DiffResult.Type == FDiffNodeEdgeLinkedToPinResult::EDiffType::NO_DIFFERENCE);
#endif
	}
	// Some link was added/removed
	else if (DialogueNodeChildrenNum < GraphNodeChildrenNum)
	{
		// Output link added, extend the number of children
		// Not handled here, as everytime we just add at the end of the array.
		// See UDialogueGraphNode_Edge::CreateConnections
	}
	else
	{
		// One Output link removed, reduce the number of linked children
		check(DialogueNodeChildrenNum > GraphNodeChildrenNum);
		if (GraphNodeChildrenNum == 0)
		{
			// All nodes were removed, node is most likely going to be removed
			DialogueNode->RemoveAllChildren();
		}
		else
		{
			// Only one
			const FDiffNodeEdgeLinkedToPinResult DiffResult = FindDifferenceBetweenNodeEdgesAndLinkedToPins();
			check(DiffResult.Type == FDiffNodeEdgeLinkedToPinResult::EDiffType::LENGTH_MISMATCH_ONE_MORE_EDGE ||
				DiffResult.Type == FDiffNodeEdgeLinkedToPinResult::EDiffType::EDGE_NOT_MATCHING_INDEX);
			DialogueNode->RemoveChildAt(DiffResult.Index);
		}
	}*/
}

void UQuestGraphNode::GetNodeContextMenuActions(UToolMenu* Menu, UGraphNodeContextMenuContext* Context) const
{
	// These actions (commands) are handled and registered in the FQuestAssetEditorToolkit class
	if (Context->Node && !Context->bIsDebugging)
	{
		// Menu for right clicking on node
		FToolMenuSection& Section = Menu->AddSection("DialogueGraphNode_BaseNodeEditCRUD");

		//Conversion Entries
		Section.AddMenuEntry(FQuestCommands::Get().ConvertObjectiveToCollect);
		Section.AddMenuEntry(FQuestCommands::Get().ConvertObjectiveToInteract);
		Section.AddMenuEntry(FQuestCommands::Get().ConvertObjectiveToKill);
		Section.AddMenuEntry(FQuestCommands::Get().ConvertObjectiveToLocate);


		Section.AddMenuEntry(FGenericCommands::Get().Delete);
		//		Section.AddMenuEntry(FGenericCommands::Get().Cut);
		Section.AddMenuEntry(FGenericCommands::Get().Copy);
		//		Section.AddMenuEntry(FGenericCommands::Get().Duplicate);

	}
}

void UQuestGraphNode::AutowireNewNode(UEdGraphPin* FromPin)
{
	// No context given, simply return
	if (FromPin == nullptr)
	{
		return;
	}

	// FromPin should not belong to this node but to the node that spawned this node.
	check(FromPin->GetOwningNode() != this);
	//check(FromPin->Direction == EGPD_Output);

	const UQuestGraphSchema* Schema = GetQuestGraphSchema();

	//Dragged from Output Pin
	if (FromPin->Direction == EGPD_Output) {

		UEdGraphPin* InputpIn = GetInputPin();

		// auto-connect from dragged pin to first compatible pin on the new node
		verify(Schema->TryCreateConnection(FromPin, InputpIn));
		FromPin->GetOwningNode()->NodeConnectionListChanged();
	}
	//Dragged from Input Pin
	else
	{
		UQuestGraphNode* checkNode = Cast<UQuestGraphNode>(FromPin->GetOuter());
		//Dragging from Input Pin of Root is not allowed
		if (checkNode && !checkNode->IsRootNode()) {
			UEdGraphPin* OutputPin = GetOutputPin();
			verify(Schema->TryCreateConnection(OutputPin, FromPin));
			OutputPin->GetOwningNode()->NodeConnectionListChanged();
		}
	}
}

/* ***** END UEdGraphNode interface ***** */


/* ----- UQuestGraphNode_Base interface ----- */

FLinearColor UQuestGraphNode::GetNodeBackgroundColor() const
{
	if (NodeIndex == INDEX_NONE)
	{
		return FLinearColor::Gray;
	}

	//TODO: Hier ggf. je nach Node-typen unterschiedliche Hintergrundfarben
	const UObjective* NodeObjective = GetMutableQuestNode();

	if (Cast<UKillObjective>(NodeObjective))
	{
		return FLinearColor::Red;
	}

	if (Cast<UCollectObjective>(NodeObjective))
	{
		return FLinearColor::Green;
	}

	if (Cast<UInteractObjective>(NodeObjective))
	{
		return FLinearColor::Blue;
	}

	if (Cast<ULocationObjective>(NodeObjective))
	{
		return FLinearColor::White;
	}


	return FLinearColor::Black;
}

bool UQuestGraphNode::HasOutputConnectionToNode(const UEdGraphNode* TargetNode) const
{
	for (UQuestGraphNode* ChildNode : GetChildNodes())
	{
		if (ChildNode == TargetNode)
		{
			return true;
		}
	}

	return false;;
}

void UQuestGraphNode::RegisterListeners()
{
	Super::RegisterListeners();


	UE_LOG(LogTemp, Warning, TEXT("Registered Node PropertyChanged"));
	//TODO: Bei convert geht das hier irgendwie kaputt...
	QuestNode->OnObjectivePropertyChanged.AddUObject(this, &UQuestGraphNode::OnNodePropertyChanged);
}

void UQuestGraphNode::SetQuestNodeDataChecked(int32 InIndex, UObjective* InNode)
{
	const UQuestAsset* Quest = GetQuestAsset();
	checkf(Quest->GetObjectives()[InIndex] == InNode, TEXT("The selected index = %d and with the Node provided does not match the Node from the Dialogue"), InIndex);

	SetQuestNodeIndex(InIndex);
	SetQuestNode(InNode);
}


/* ***** END UQuestGraphNode_Base interface ***** */

/* ----- Begin own functions ----- */

void UQuestGraphNode::ApplyCompilerWarnings()
{
	ClearCompilerMessage();

	// Is Orphan node
	if (!IsRootNode() && GetInputPin()->LinkedTo.Num() == 0)
	{
		SetCompilerWarningMessage(TEXT("Node has no input connections (orphan). It will not be accessible from anywhere"));
	}
	/*else if (QuestNode->GetNodeOpenChildren_DEPRECATED().Num() > 0)
	{
		// Has open children :O
		SetCompilerWarningMessage(TEXT("Node has invalid (open) edges in its DialogueNode"));
	}*/
}

int32 UQuestGraphNode::EstimateNodeWidth() const
{
	constexpr int32 EstimatedCharWidth = 6;
	// Check which is bigger, and use that
	const FString NodeTitle = GetNodeTitle(ENodeTitleType::FullTitle).ToString();
	const FString NodeText = QuestNode->GetObjectiveDescription().ToString();

	int32 Result;
	FString ResultFromString;
	if (NodeTitle.Len() > NodeText.Len())
	{
		Result = NodeTitle.Len() * EstimatedCharWidth;
		ResultFromString = NodeTitle;
	}
	else
	{
		Result = NodeText.Len() * EstimatedCharWidth;
		ResultFromString = NodeText;
	}

	if (const UFont* Font = GetDefault<UEditorEngine>()->EditorFont)
	{
		Result = Font->GetStringSize(*ResultFromString);
	}

	return Result;
}

void UQuestGraphNode::CheckQuestNodeIndexMatchesNode() const
{
#if DO_CHECK
	if (!IsRootNode())
	{
		const UQuestAsset* Quest = GetQuestAsset();
		checkf(Quest->GetObjectives()[NodeIndex] == QuestNode, TEXT("The NodeIndex = %d with QuestNode does not match the Node from the Objective at the same index"), NodeIndex);
	}
#endif
}

void UQuestGraphNode::CheckQuestNodeSyncWithGraphNode(bool bStrictCheck) const
{
	//TODO (Testen): Nötig? Edges und so?
}

TArray<UQuestGraphNode*> UQuestGraphNode::GetParentNodes() const
{
	//check(HasParentNode());
	TArray<UQuestGraphNode*> ParentNodes;

	//Nodes are directly connected, without custom edges in between
	UEdGraphPin* inputPin = GetInputPin();

	for (int i = 0; i < inputPin->LinkedTo.Num(); i++)
	{
		ParentNodes.Add(CastChecked<UQuestGraphNode>(inputPin->LinkedTo[i]->GetOwningNode()));
	}

	return ParentNodes;
}

TArray<UQuestGraphNode*> UQuestGraphNode::GetChildNodes() const
{
	TArray<UQuestGraphNode*> ChildNodes;
	//TODO (Prüfen): Dieser check ist nicht sinnvoll, da es fälle gibt, in denen GetChildNodes() aufgerufen wird und dann geprüft wird, ob diese leer sind o.ä.
	//check(HasChildNode());

	UEdGraphPin* outputPin = GetOutputPin();

	for (int i = 0; i < outputPin->LinkedTo.Num(); i++)
	{
		ChildNodes.Add(CastChecked<UQuestGraphNode>(outputPin->LinkedTo[i]->GetOwningNode()));
	}

	return ChildNodes;
}

struct FCompareNodeYLocation
{
	FORCEINLINE bool operator()(const TPair<UEdGraphPin*, UObjective*>& A, const TPair<UEdGraphPin*, UObjective*>& B) const
	{
		const UEdGraphNode* NodeA = A.Key->GetOwningNode();
		const UEdGraphNode* NodeB = B.Key->GetOwningNode();
		return NodeA->NodePosY != NodeB->NodePosY ? NodeA->NodePosY < NodeB->NodePosY : NodeA->NodePosY < NodeB->NodePosY;
	}
};

void UQuestGraphNode::SortChildrenBasedOnYLocation()
{	
	// Holds an array of synced pairs, each pair corresponds to a linked to output pin and corresponding objective
	TArray<TPair<UEdGraphPin*, UObjective*>> SyncedArray;

	UEdGraphPin* OutputPin = GetOutputPin();
	const TArray<UEdGraphPin*> ChildPins = OutputPin->LinkedTo;
	const TArray<UObjective*>& ChildObjectives = QuestNode->GetNodeChildren();	
	checkf(ChildPins.Num() == ChildObjectives.Num(), TEXT("Child Pins (%d) and Child Objectives (%d) do not have the same amount, but should have!"), ChildPins.Num(), ChildObjectives.Num());

	// Step 1. Construct the synced array
	const int32 ChildrenNum = ChildPins.Num();
	SyncedArray.Reserve(ChildrenNum);
	for (int32 ChildIndex = 0; ChildIndex < ChildrenNum; ChildIndex++)
	{
		SyncedArray.Emplace(ChildPins[ChildIndex], ChildObjectives[ChildIndex]);
	}

	// Step 2. Sort the synced array
	SyncedArray.Sort(FCompareNodeYLocation());

	// Step 3. Reconstruct the output pins/edges from the sorted synced array
	OutputPin->LinkedTo.Empty();
	QuestNode->RemoveAllChildren();
	for (const TPair<UEdGraphPin*, UObjective*>& SyncedPair : SyncedArray)
	{
		OutputPin->LinkedTo.Add(SyncedPair.Key);
		QuestNode->AddNodeChild(SyncedPair.Value);
	}
}

void UQuestGraphNode::SortParentsBasedOnYLocation()
{
	// Holds an array of synced pairs, each pair corresponds to a linked to output pin and corresponding objective
	TArray<TPair<UEdGraphPin*, UObjective*>> SyncedArray;

	UEdGraphPin* InputPin = GetInputPin();
	const TArray<UEdGraphPin*> ParentPins = InputPin->LinkedTo;
	const TArray<UObjective*>& ParentObjectives = QuestNode->GetNodeParents();
	//TODO: Wenn das hier nicht gleich ist, dann wurde den Eltern wohl kein Knoten hinzugefügt... 
	checkf(ParentPins.Num() == ParentObjectives.Num(), TEXT("Parent Pins (%d) and Parent Objectives (%d) do not have the same amount, but should have!"), ParentPins.Num(), ParentObjectives.Num());

	// Step 1. Construct the synced array
	const int32 ParentNum = ParentPins.Num();
	SyncedArray.Reserve(ParentNum);
	for (int32 ParentIndex = 0; ParentIndex < ParentNum; ParentIndex++)
	{
		SyncedArray.Emplace(ParentPins[ParentIndex], ParentObjectives[ParentIndex]);
	}

	// Step 2. Sort the synced array
	SyncedArray.Sort(FCompareNodeYLocation());

	// Step 3. Reconstruct the output pins/edges from the sorted synced array
	InputPin->LinkedTo.Empty();
	QuestNode->RemoveAllParents();
	for (const TPair<UEdGraphPin*, UObjective*>& SyncedPair : SyncedArray)
	{
		InputPin->LinkedTo.Add(SyncedPair.Key);
		QuestNode->AddNodeParent(SyncedPair.Value);
	}
}


void UQuestGraphNode::OnNodePropertyChanged(const FPropertyChangedEvent& PropertyChangedEvent)
{
	if (!PropertyChangedEvent.Property)
	{
		return;
	}

	//TODO: Perform List creation somewhere on initialization
	//Create a List of all Properties which should lead to a refresh when being changed.
	TSet<FName> PropertiesToConsider;
	PropertiesToConsider.Add(GET_MEMBER_NAME_CHECKED(UObjective, Title));

	//Only Refresh the Graphview if there is just one Node selected (Prevent crashes)
	int SelectedNodesAmount = FQuestEditorUtilities::GetQuestEditorForGraph(GetGraph())->GetPreviousSelectedNodesAmount();
	const FName PropertyName = PropertyChangedEvent.Property->GetFName();
	if (PropertiesToConsider.Contains(PropertyName) && SelectedNodesAmount <= 1)
	{
		FQuestEditorUtilities::SimpleRefresh(GetGraph(), true);
	}

}


void UQuestGraphNode::ResetQuestNodeOwner()
{
	if (!QuestNode)
	{
		return;
	}

	UQuestAsset* Quest = GetQuestAsset();

	// Ensures DialogueNode is owned by the Dialogue
	if (QuestNode->GetOuter() != Quest)
	{
		QuestNode->Rename(nullptr, Quest, REN_DontCreateRedirectors);
	}

	// Set up the back pointer for newly created dialogue node nodes
	QuestNode->SetGraphNode(this);
}

#undef LOCTEXT_NAMESPACE
