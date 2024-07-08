//TODO: Copyright
#pragma once

#include "CoreMinimal.h"

#if WITH_EDITOR
#include "EdGraph/EdGraphNode.h"

class UEdGraph;
class UQuestAsset;
//TODO: Brauchen wir UQuestNode?

/**
 * Interface for dialogue graph interaction with the DlgSystemEditor module.
 * See DlgDialogueEditorModule.h (in the DlgSystemEditor) for the implementation of this interface.
 */
class QUESTASSET_API IQuestEditorAccess
{
public:
	virtual ~IQuestEditorAccess() {}

	// Updates the graph node edges data to match the quest data
	virtual void UpdateGraphNodeEdges(UEdGraphNode* GraphNode) = 0;

	// Creates a new quest graph.
	virtual UEdGraph* CreateNewQuestGraph(UQuestAsset* QuestAsset) const = 0;

	// Compiles the quest nodes from the graph nodes. Meaning it transforms the graph data -> (into) quest data.
	virtual void CompileQuestNodesFromGraphNodes(UQuestAsset* Quest) const = 0;

	// Removes all nodes from the graph.
	virtual void RemoveAllGraphNodes(UQuestAsset* Quest) const = 0;

	// Tells us if the number of quest nodes matches with the number of graph nodes (corresponding to quests).
	virtual bool AreQuestNodesInSyncWithGraphNodes(UQuestAsset* QuestAsset) const = 0;

	// Updates the Dialogue to match the version UseOnlyOneOutputAndInputPin
	virtual void UpdateQuestToVersion_UseOnlyOneOutputAndInputPin(UQuestAsset* Dialogue) const = 0;

	//TODO: Brauchen wir das und brauchen wir UQuestNode?
	//TODO: Objective oder Object?
	// Tries to set the new outer for Object to the closes UQuestNode from UEdGraphNode
	virtual void SetNewOuterForObjectFromGraphNode(UObject* Object, UEdGraphNode* GraphNode) const = 0;
};



#endif //#if WITH_EDITOR
