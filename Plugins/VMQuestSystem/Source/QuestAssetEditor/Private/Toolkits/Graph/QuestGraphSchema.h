//TODO: Copyright
#pragma once

#include "EdGraph/EdGraphSchema.h"
#include "QuestAsset/Private/Objective.h"

#include "QuestGraphSchema.generated.h"

class UQuestGraphNode;
class UGraphNodeContextMenuContext;
class UToolMenu;
class FMenuBuilder;
class FSlateWindowElementList;
class UEdGraph;


UCLASS()
class UQuestGraphSchema : public UEdGraphSchema
{
	GENERATED_BODY()

public:
	// Helper method to add items valid to the palette list 
	void GetPaletteActions(FGraphActionMenuBuilder& ActionMenuBuilder) const;

	
	/** Check whether connecting these pins would cause a loop
	 *	Loop means a connection connects in and output pin directly */
	bool ConnectionCausesLoop(const UEdGraphPin* InputPin, const UEdGraphPin* OutputPin) const;

	/* ---- EdGraphSchema Interface ---- */

	/**
	 * Get all actions that can be performed when right clicking on a graph or drag-releasing on a graph from a pin
	 *
	 * @param [in,out]	ContextMenuBuilder	The context (graph, dragged pin, etc...) and output menu builder.
	 */
	void GetGraphContextActions(FGraphContextMenuBuilder& ContextMenuBuilder) const override;

	/**
	* Gets actions that should be added to the right-click context menu for a node or pin
	* @param	Menu				The menu to append actions to.
	* @param	Context				The menu's context.
	*/
	void GetContextMenuActions(UToolMenu* Menu, UGraphNodeContextMenuContext* Context) const override;

	/**
	 * Populate new graph with any default nodes
	 *
	 * @param	Graph			Graph to add the default nodes to
	 */
	void CreateDefaultNodesForGraph(UEdGraph& Graph) const override;

	/** Break links on this pin and create links instead on MoveToPin */
	FPinConnectionResponse MovePinLinks(
		UEdGraphPin& MoveFromPin,
		UEdGraphPin& MoveToPin,
		bool bIsIntermediateMove = false,
		bool bNotifyLinkedNodes = false
	) const override;

	/** Copies pin links from one pin to another without breaking the original links */
	FPinConnectionResponse CopyPinLinks(UEdGraphPin& CopyFromPin, UEdGraphPin& CopyToPin, bool bIsIntermediateCopy = false) const override;

	/**
	 * Determine if a connection can be created between two pins.
	 *
	 * @param	PinA	The first pin.
	 * @param	PinB	The second pin.
	 *
	 * @return	An empty string if the connection is legal, otherwise a message describing why the connection would fail.
	 */
	const FPinConnectionResponse CanCreateConnection(const UEdGraphPin* PinA, const UEdGraphPin* PinB) const override;

	
	/**
	 * Try to make a connection between two pins.
	 *
	 * @param	PinA	The first pin.
	 * @param	PinB	The second pin.
	 *
	 * @return	True if a connection was made/broken (graph was modified); false if the connection failed and had no side effects.
	 */
	bool TryCreateConnection(UEdGraphPin* PinA, UEdGraphPin* PinB) const override;

	/**
	 * Try to create an automatic cast or other conversion node node to facilitate a connection between two pins.
	 * It makes the cast node, a connection between A and the cast node, and a connection from the cast node to B.two
	 * This method is called when a connection is made where CanCreateConnection returned bCanAutoConvert.
	 *
	 *
	 * @param	PinA	The first pin.
	 * @param	PinB	The second pin.
	 *
	 * @return	True if a cast node and connection were made; false if the connection failed and had no side effects.
	 */
	bool CreateAutomaticConversionNodeAndConnections(UEdGraphPin* PinA, UEdGraphPin* PinB) const override;

	/** If we should disallow viewing and editing of the supplied pin */
	bool ShouldHidePinDefaultValue(UEdGraphPin* Pin) const override;

	/**
	 * Breaks all links from/to a single node
	 *
	 * @param	TargetNode	The node to break links on
	 */
	void BreakNodeLinks(UEdGraphNode& TargetNode) const override;

	/**
	 * Breaks all links from/to a single pin
	 *
	 * @param	TargetPin	The pin to break links on
	 * @param	bSendsNodeNotifcation	whether to send a notification to the node post pin connection change
	 */
	void BreakPinLinks(UEdGraphPin& TargetPin, bool bSendsNodeNotifcation) const override;

	/**
	 * Breaks the link between two nodes.
	 *
	 * @param	SourcePin	The pin where the link begins.
	 * @param	TargetPin	The pin where the link ends.
	 */
	void BreakSinglePinLink(UEdGraphPin* SourcePin, UEdGraphPin* TargetPin) const override;

	/** Called when asset(s) are dropped onto the specified node */
	void DroppedAssetsOnGraph(const TArray<FAssetData>& Assets, const FVector2D& GraphPosition, UEdGraph* Graph) const override;

	/** Called when asset(s) are dropped onto the specified node */
	void DroppedAssetsOnNode(const TArray<FAssetData>& Assets, const FVector2D& GraphPosition, UEdGraphNode* Node) const override;

	/**
	 * Returns the currently selected graph node count
	 *
	 * @param	Graph			The active graph to find the selection count for
	 */
	//TODO: FQuestEditorUtilities
	//int32 GetNodeSelectionCount(const UEdGraph* Graph) const override { return FDialogueEditorUtilities::GetSelectedNodes(Graph).Num(); }

	/**
	 * When a node is removed, this method determines whether we should remove it immediately or use the old (slower) code path that
	 * results in all node being recreated:
	 */
	bool ShouldAlwaysPurgeOnModification() const override { return true; }

	/** Returns schema action to create comment from implemention */
	//TODO: FNewComment_QuestGraphSchemaAction
	/*TSharedPtr<FEdGraphSchemaAction> GetCreateCommentAction() const override
	{
		return TSharedPtr<FEdGraphSchemaAction>(static_cast<FEdGraphSchemaAction*>(new FNewComment_DialogueGraphSchemaAction));
	}*/

	/* Returns new FConnectionDrawingPolicy from this schema */
	//TODO: FQuestGraphConnectionDrawingPolicy
	/*
	FConnectionDrawingPolicy* CreateConnectionDrawingPolicy(
		int32 InBackLayerID,
		int32 InFrontLayerID,
		float InZoomFactor,
		const FSlateRect& InClippingRect,
		FSlateWindowElementList& InDrawElements,
		UEdGraph* InGraphObj
	) const override
	{
		return new FDialogueGraphConnectionDrawingPolicy(InBackLayerID, InFrontLayerID, InZoomFactor, InClippingRect, InDrawElements, InGraphObj);
	}*/

	/* *****  END EdGraphSchema  ****** */

	/* ---- Own Functions ----- */

	/**
	 * Breaks all links from/to a single pin
	 *
	 * @param	FromPin		The pin to break links from
	 * @param	ToPin		The pin we are breaking links to
	 * @param	bSendsNodeNotifcation	whether to send a notification to the node post pin connection change
	 */
	void BreakLinkTo(UEdGraphPin* TargetPin, UEdGraphPin* ToPin, bool bSendsNodeNotifcation) const;

private:
	/** Adds action for creating a comment */
	void GetCommentAction(FGraphActionMenuBuilder& ActionMenuBuilder, const UEdGraph* CurrentGraph = nullptr) const;
		
	/** Adds actions for creating every type of Objective */
	void GetAllQuestNodeActions(FGraphActionMenuBuilder& ActionMenuBuilder) const;

	/** Generates a list of all available UObjective classes */
	static void InitQuestNodeClasses();

public:
	// Allowed PinType.PinCategory values
	static const FName PIN_CATEGORY_Input;
	static const FName PIN_CATEGORY_Output;

	// Categories for actions
	static const FText NODE_CATEGORY_Quest;
	static const FText NODE_CATEGORY_Graph;	

private:
	/** A list of all available UObjective classes */	
	static TArray<TSubclassOf<UObjective>> QuestNodeClasses;

	/** Whether the list of UObjective classes has been populated */	
	static bool bQuestNodeClassesInitialized;
};