//TODO: Copyright
#pragma once


#include "CoreTypes.h"
#include "QuestGraphNode_Base.h"
#include "UObject/ObjectMacros.h"

#include "QuestGraphNode.generated.h"

class UEdGraphPin;
class UToolMenu;
class UGraphNodeContextMenuContext;

/** Result for a single difference between the Quest Node Edges and LinkedTo Array of the output pins */
struct FDiffNodeEdgeLinkedToPinResult
{
	enum EDiffType
	{
		NO_DIFFERENCE = 0,

		// A result of DoesEdgeMatchEdgeIndex
		EDGE_NOT_MATCHING_INDEX,

		// The length of the arrays mismatch, there is one more edge (located at the end) than pin connection
		LENGTH_MISMATCH_ONE_MORE_EDGE,

		// The length of the arrays mismatch, there is one more pin connection (located at the end) than edges
		LENGTH_MISMATCH_ONE_MORE_PIN_CONNECTION,

		// Diff type not supported
		NOT_SUPPORTED
	};


	/** Tells us the type of diff */
	EDiffType Type = EDiffType::NO_DIFFERENCE;

	/**
	 * Depending on the Type of diff this index can mean diferent things:
	 * - EDGE_NOT_MATCHING_INDEX
	 *		- represents the Index number of both arrays that is different
	 * - LENGTH_MISMATCH_ONE_MORE_EDGE
	 *		- represents the index (of the one more edge) that does not exist in the LinkedTo array but exists in the Edges array
	 * - LENGTH_MISMATCH_ONE_MORE_PIN_CONNECTION
	 *		- represents the index (of the one more pin connection) that does not exist in the Edges array but exists in the LinkedTo array
	 */
	int32 Index = INDEX_NONE;

	/** Some error message if any. */
	FString Message;
};

UCLASS()
class UQuestGraphNode : public UQuestGraphNode_Base
{
	GENERATED_BODY()

public:
	/* ----- UObject Interface ----- */

	/**
	 * Do any object-specific cleanup required immediately after loading an object,
	 * and immediately after any undo/redo.
	 */
	void PostLoad() override;

	/**
	 * Called after importing property values for this object (paste, duplicate or .t3d import)
	 * Allow the object to perform any cleanup for properties which shouldn't be duplicated or
	 * are unsupported by the script serialization
	 */
	void PostEditImport() override;

	/**
	 * Called when a property on this object has been modified externally
	 *
	 * @param PropertyChangedEvent the property that was modified
	 */
	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

	/**
	 * This alternate version of PostEditChange is called when properties inside structs are modified.  The property that was actually modified
	 * is located at the tail of the list.  The head of the list of the FStructProperty member variable that contains the property that was modified.
	 */
	void PostEditChangeChainProperty(struct FPropertyChangedChainEvent& PropertyChangedEvent) override;

	/**
	 * Note that the object will be modified.  If we are currently recording into the
	 * transaction buffer (undo/redo), save a copy of this object into the buffer and
	 * marks the package as needing to be saved.
	 *
	 * @param	bAlwaysMarkDirty	if true, marks the package dirty even if we aren't
	 *								currently recording an active undo/redo transaction
	 * @return true if the object was saved to the transaction buffer
	 */
	bool Modify(bool bAlwaysMarkDirty = true) override;

	/* ***** END UObject Interface ***** */


	/* ----- UEdGraphNode interface ----- */

	/** Gets the name of this node, shown in title bar */
	FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;

	/** Gets the tooltip to display when over the node */
	FText GetTooltipText() const override;
	FString GetDocumentationExcerptName() const override;

	/** Whether or not this node can be safely duplicated (via copy/paste, etc...) in the graph. */
	bool CanDuplicateNode() const override { return true;/*return !IsRootNode();*/ } //After all: Why shouldn't we copy it...

	/** Whether or not this node can be deleted by user action. */
	bool CanUserDeleteNode() const override { return !IsRootNode(); }

	/** Perform any steps necessary prior to copying a node into the paste buffer */
	void PrepareForCopying() override;

	/**
	 * Called when something external to this node has changed the connection list of any of the pins in the node
	 *   - Different from PinConnectionListChanged as this is called outside of any loops iterating over our pins allowing
	 *	 us to do things like reconstruct the node safely without trashing pins we are already iterating on
	 *   - Typically called after a user induced action like making a pin connection or a pin break
	 */
	void NodeConnectionListChanged() override
	{
		CheckQuestNodeSyncWithGraphNode(true);
		ApplyCompilerWarnings();
	}

	/** Called when the connection list of one of the pins of this node is changed in the editor */
	void PinConnectionListChanged(UEdGraphPin* Pin) override;

	/** Gets a list of actions that can be done to this particular node */
	void GetNodeContextMenuActions(UToolMenu* Menu, UGraphNodeContextMenuContext* Context) const override;

	/**
	 * Autowire a newly created node.
	 *
	 * @param FromPin	The source pin that caused the new node to be created (typically a drag-release context menu creation).
	 */
	void AutowireNewNode(UEdGraphPin* FromPin) override;

	/* ***** END UEdGraphNode interface ***** */



	/* ----- UQuestGraphNode_Base interface ----- */

	/** Checks whether an input connection can be added to this node */
	bool CanHaveInputConnections() const override { return NodeIndex != INDEX_NONE && !IsRootNode(); }

	/** Checks whether an output connection can be added from this node */
	bool CanHaveOutputConnections() const override { return true; }

	/** Checks if this node has a output connection to the TargetNode. */
	bool HasOutputConnectionToNode(const UEdGraphNode* TargetNode) const override;

	/** Gets the background color of this node. */
	FLinearColor GetNodeBackgroundColor() const override;

	/** Perform any fixups (deep copies of associated data, etc...) necessary after a node has been copied in the editor. */
	void PostCopyNode() override;

	/** Perform all checks */
	void CheckAll() const override
	{
#if DO_CHECK
		Super::CheckAll();
		check(IsQuestNodeSet());
		CheckQuestNodeIndexMatchesNode();
		CheckQuestNodeSyncWithGraphNode(true);
#endif
	}

	/** Is this the undeletable root node */
	virtual bool IsRootNode() const { return NodeIndex == 0; }

	/* ***** END UQuestGraphNode_Base interface ***** */



	/* ----- Begin own functions ----- */

	/** Does this node has any enter conditions? */
	bool HasEnterConditions() const
	{
		return QuestNode != nullptr;		
	}

	/** Does this node has any enter events? */
	bool HasEnterEvents() const
	{
		return QuestNode ? QuestNode->HasAnyEnterEvents() : false;
	}

	/** Gets the node depth in the graph. */
	int32 GetNodeDepth() const { return NodeDepth; }

	/** Sets the new node depth. */
	void SetNodeDepth(int32 NewNodeDepth) { NodeDepth = NewNodeDepth; }

	/** Sets the Quest Node. */
	virtual void SetQuestNode(UObjective* InNode)
	{
		QuestNode = InNode;
		QuestNode->SetFlags(RF_Transactional);
		QuestNode->SetGraphNode(this);
		RegisterListeners();
	}

	/** Sets the Quest node index number, this represents the index from the QuestAsset objectives Array */
	virtual void SetQuestNodeIndex(int32 InIndex)
	{
		check(InIndex > INDEX_NONE);
		NodeIndex = InIndex;
	}

	// Where should the edges pointing to this node be positioned at
	// NOTE: we use this because otherwise the edges don't get rendered
	FIntPoint GetDefaultEdgePosition() const { return GetPosition() + FIntPoint(5, 5); }

	/**
	 * The same SetDialogueNodeIndex and SetDialogueNode only that it sets them both at once and it does some sanity checking
	 * such as verifying the index is valid in the Dialogue node and that the index corresponds to this InNode.
	 */
	void SetQuestNodeDataChecked(int32 InIndex, UObjective* InNode);

	/** Gets the copy of the QuestNode stored by this graph node */
	template <typename QuestNodeType>
	const QuestNodeType& GetQuestNode() const { return *CastChecked<QuestNodeType>(QuestNode); }

	/** Gets the copy of the QuestNote stored by this graph node as a mutable pointer */
	template <typename QuestNodeType>
	QuestNodeType* GetMutableQuestNode() { return CastChecked<QuestNodeType>(QuestNode); }

	// Specialization for the methods above  (by overloading) for the base type UObjective type so that we do not need to cast
	const UObjective& GetQuestNode() const { return *QuestNode; }
	UObjective* GetMutableQuestNode() const { return QuestNode; }

	/** Tells us if the Quest Node is valid non null. */
	bool IsQuestNodeSet() const { return QuestNode != nullptr; }

	/** Gets the Quest node index number for the QuestAsset objectives Array */
	virtual int32 GetQuestNodeIndex() const { return NodeIndex; }
		
	/** Checks the node for warnings and applies the compiler warnings messages */
	void ApplyCompilerWarnings();

	/** Estimate the width of this Node from the length of its content */
	int32 EstimateNodeWidth() const;

	/** Checks QuestAsset.Objectives[NodeIndex] == QuestNode */
	void CheckQuestNodeIndexMatchesNode() const;

	/** Helper function to check if the DialogueNode.Children matches with the Pins of the graph node */
	void CheckQuestNodeSyncWithGraphNode(bool bStrictCheck = false) const;

	/** Gets the parent nodes that are connected to the input pin. This handles the proxy connection to the UDialogueGraphNode_Edge.  */
	TArray<UQuestGraphNode*> GetParentNodes() const;

	/** Gets the child nodes that are connected from the output pin. This handles the proxy connection to the UDialogueGraphNode_Edge.  */
	TArray<UQuestGraphNode*> GetChildNodes() const;

	/** Rearranges the children (output pin, connections) based on the Y location on the graph. */
	void SortChildrenBasedOnYLocation();

	/** Rearranges the parents (output pin, connections) based on the Y location on the graph. */
	void SortParentsBasedOnYLocation();
	
	/** Should we force hide this node? */
	bool GetForceHideNode() const { return bForceHideNode; }

	/** Forcefully hide/show this node and all edges that are coming into it. */
	void SetForceHideNode(bool bHide) { bForceHideNode = bHide; }

	/** Should this node be drawn? */
	bool ShouldDrawNode() const { return !bForceHideNode; }

	/** Helper constants to get the names of some properties. Used by the DlgSystemEditor module. */
	static FName GetMemberNameQuestNode() { return GET_MEMBER_NAME_CHECKED(UQuestGraphNode, QuestNode); }
	static FName GetMemberNameNodeIndex() { return GET_MEMBER_NAME_CHECKED(UQuestGraphNode, NodeIndex); }

	/** Does this node have a parent node? */
	bool HasParentNode() const
	{
		if (!IsRootNode() && HasInputPin())
		{
			UEdGraphPin* InputPin = GetInputPin();
			return InputPin->LinkedTo.Num() >= 1 && InputPin->LinkedTo[0] != nullptr &&
				InputPin->LinkedTo[0]->GetOwningNodeUnchecked() != nullptr;
		}

		return false;
	}

	/** Does this node has a child node? */
	bool HasChildNode() const
	{
		if (HasOutputPin())
		{
			UEdGraphPin* OutputPin = GetOutputPin();
			return OutputPin->LinkedTo.Num() >= 1 && OutputPin->LinkedTo[0] != nullptr &&
				OutputPin->LinkedTo[0]->GetOwningNodeUnchecked() != nullptr;
		}

		return false;
	}
	

	/* ----- UQuestGraphNode_Base interface ----- */
protected:
	/** Creates the input pin for this node. */
	virtual void CreateInputPin()
	{		
		static const FName PinName(TEXT("Input Pin"));
		FCreatePinParams PinParams;
		PinParams.Index = INDEX_PIN_Input;
		CreatePin(EGPD_Input, UQuestGraphSchema::PIN_CATEGORY_Input, PinName, PinParams);
	}

	/** Creates the output pin for this node. */
	virtual void CreateOutputPin()
	{
		static const FName PinName(TEXT("Output Pin"));
		FCreatePinParams PinParams;
		PinParams.Index = INDEX_PIN_Output;
		CreatePin(EGPD_Output, UQuestGraphSchema::PIN_CATEGORY_Output, PinName, PinParams);

		// This enables or disables dragging of the pin from the Node, see SGraphPin::OnPinMouseDown for details
		//GetOutputPin()->bNotConnectable = IsEndNode();
	}

	/** Registers all the listener this class listens to. */
	void RegisterListeners() override;
	

	/* ----- Begin own functions ----- */
	
	void OnNodePropertyChanged(const FPropertyChangedEvent& PropertyChangedEvent);

	/** Make sure the QuestNode is owned by the Quest */
	void ResetQuestNodeOwner();

protected:
	/** The Quest Node this graph node references.  */
	UPROPERTY(VisibleAnywhere, Instanced, Category = QuestGraphNode, Meta = (ShowOnlyInnerProperties))
		UObjective* QuestNode;

	/** The Quest Node index in the Quest (array of objectives) this node represents. Starts with 0 as that's always the root!*/	
	UPROPERTY(VisibleAnywhere, Category = QuestGraphNode)
		int32 NodeIndex = 0;

	// Indicates the distance from the start node. This is only set after the graph is compiled.
	UPROPERTY()
		int32 NodeDepth = INDEX_NONE;

	/**
	 * Forcefully hide this node from the graph.
	 */
	bool bForceHideNode = false;

	/* ****** END own functions ****** */

};