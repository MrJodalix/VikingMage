//TODO: Copyright
#pragma once

#include "CoreMinimal.h"
#include "EdGraph/EdGraph.h"

#include "QuestAsset.h"

#include "QuestGraph.generated.h"


class UQuestGraphSchema;
class UQuestGraphNode_Base;
class UQuestGraphNode;

UCLASS()
class UQuestGraph : public UEdGraph
{
	GENERATED_BODY()

public:

	/* ---- Begin UObject Interface ---- */
	/**
	* Note that the object will be modified. If we are currently recording into the
	* transaction buffer (undo/redo), save a copy of this object into the buffer and
	* marks the package as needing to be saved.
	*
	* @param	bAlwaysMarkDirty	if true, marks the package dirty even if we aren't
	*								currently recording an active undo/redo transaction
	* @return true if the object was saved to the transaction buffer
	*/
	bool Modify(bool bAlwaysMarkDirty = true) override;

	/* ---- Begin UEdGraph ---- */
	/** Remove a node from this graph. Variant of UEdGraph::RemoveNode */
	bool RemoveGraphNode(UEdGraphNode* NodeToRemove);

	/* ---- Own Methods ---- */

	/** Gets the DlgDialogue that contains this graph */
	UQuestAsset* GetQuestAsset() const
	{
		// Unreal engine magic, get the object that owns this graph, that is our QuestAsset.
		return CastChecked<UQuestAsset>(GetOuter());
	}

	//TODO: UQuestGraphNode_Root
	/** Gets the root graph node of this graph */
	UQuestGraphNode* GetRootGraphNode();

	/** Gets all the graph nodes of this  Graph */
	const TArray<UEdGraphNode*>& GetAllGraphNodes() const { return Nodes; }

	/** Gets the all the dialogue graph nodes (that inherit from UQuestNodeGraphNode_Base). Includes Root node. */

	TArray<UQuestGraphNode_Base*> GetAllBaseQuestGraphNodes() const;

	/** Gets the all the dialogue graph nodes (that inherit from UQuestGraphNode). Includes Root node. */
	TArray<UQuestGraphNode*> GetAllQuestGraphNodes() const;

	/** Gets the all the dialogue graph nodes (that inherit from UQuestGraphNode_Edge). */
	//TODO: UQuestGraphNode_Edge
	//TArray<UQuestGraphNode_Edge*> GetAllEdgeDialogueGraphNodes() const;

	/** Creates the graph nodes from the Dialogue that contains this graph */
	void CreateGraphNodesFromQuest();

	/** Creates all the links between the graph nodes from the Dialogue nodes */
	void LinkGraphNodesFromQuest();

	/** Automatically reposition all the nodes in the graph. */
	void AutoPositionGraphNodes();

	/** Remove all nodes from the graph. Without notifying anyone. This operation is atomic to the graph */
	void RemoveAllNodes();

	/** Helper method to get directly the Dialogue Graph Schema */
	const UQuestGraphSchema* GetQuestGraphSchema() const;

private:
	UQuestGraph(const FObjectInitializer& ObjectInitializer);

	// Link the specified node to all it's children	
	void LinkGraphNodeToChildren(
		const TArray<UObjective*>& NodesObjective,
		const UObjective& NodeObjective,
		UQuestGraphNode* NodeGraph
	) const;

	UPROPERTY()
		UQuestGraphNode* RootNode = nullptr;

};