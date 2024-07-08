//TODO: Copyright
#pragma once

#include "ConnectionDrawingPolicy.h"
#include "PropertyHandle.h"
#include "EdGraphNode_Comment.h"
#include "Toolkits/Graph/QuestGraph.h"
#include "QuestGraphNode.h"
#include "QuestGraphNode_Base.h"


enum class EQuestBlueprintOpenType : uint8
{
	None = 0,
	Function,
	Event
};

////////////////////////////////////////
/* FQuestEditorUtilities */

class UQuestAsset;
class UEdGraphSchema;
class UQuestGraphNode;
class UEdGraph;
class FSlateRect;
class UK2Node_Event;

class FQuestEditorUtilities
{
public:
	/** Spawns a GraphNode in the specified ParentGraph and at Location. */
	template <typename GraphNodeType>
	static GraphNodeType* SpawnGraphNodeFromTemplate(UEdGraph* ParentGraph, const FIntPoint& Location, bool bSelectNewNode = true)
	{
		FGraphNodeCreator<GraphNodeType> NodeCreator(*ParentGraph);
		GraphNodeType* GraphNode = NodeCreator.CreateUserInvokedNode(bSelectNewNode);
		NodeCreator.Finalize(); // Calls on the node: CreateNewGuid, PostPlacedNewNode, AllocateDefaultPins
		GraphNode->NodePosX = Location.X;
		GraphNode->NodePosY = Location.Y;
		GraphNode->SetFlags(RF_Transactional);

		return GraphNode;
	}

	// Loads all quests into memory and checks the GUIDs for duplicates
	static void LoadAllQuestsAndCheckGUIDs();

	/** Get the bounding area for the currently selected nodes
	 *
	 * @param Graph The Graph we are finding bounds for
	 * @param Rect Final output bounding area, including padding
	 * @param Padding An amount of padding to add to all sides of the bounds
	 *
	 * @return false if nothing is selected
	 */
	static bool GetBoundsForSelectedNodes(const UEdGraph* Graph, FSlateRect& Rect, float Padding = 0.0f);

	/** Refreshes the details panels for the editor of the specified Graph. */
	static void RefreshDetailsView(const UEdGraph* Graph, bool bRestorePreviousSelection);

	// Refresh the viewport and property/details pane
	static void Refresh(const UEdGraph* Graph, bool bRestorePreviousSelection);

	// Simple Refresh without Clearing the viewport selection
	static void SimpleRefresh(const UEdGraph* Graph, bool bRestorePreviousSelection);
	
	/** Gets the nodes that are currently selected */
	static const TSet<UObject*> GetSelectedNodes(const UEdGraph* Graph);

	/** Helper function to remove the provided node from it's graph. Returns true on success, false otherwise. */
	static bool RemoveNode(UEdGraphNode* NodeToRemove);

		
	/**
	 * Creates a new empty graph.
	 *
	 * @param	ParentScope		The outer of the new graph (typically a blueprint).
	 * @param	GraphName		Name of the graph to add.
	 * @param	SchemaClass		Schema to use for the new graph.
	 *
	 * @return	nullptr if it fails, else ther new created graph
	 */
	static UEdGraph* CreateNewGraph(
		UObject* ParentScope,
		FName GraphName,
		TSubclassOf<UEdGraph> GraphClass,
		TSubclassOf<UEdGraphSchema> SchemaClass
	);
	
	/** Tells us if the number of dialogue nodes matches with the number of graph nodes (corresponding to dialogues). */
	static bool AreQuestNodesInSyncWithGraphNodes(const UQuestAsset* Quest);

	// Tries to get the closest UDlgNode for a  UEdGraphNode
	static UObjective* GetClosestNodeFromGraphNode(UEdGraphNode* GraphNode);

	/** Gets the Dialogue from the Graph */
	static UQuestAsset* GetQuestForGraph(const UEdGraph* Graph)
	{
		return CastChecked<UQuestGraph>(Graph)->GetQuestAsset();
	}
	
	// Gets the first element from a set. From https://answers.unrealengine.com/questions/332443/how-to-get-the-firstonly-element-in-tset.html
	template <typename SetType>
	static typename TCopyQualifiersFromTo<SetType, typename SetType::ElementType>::Type* GetFirstSetElement(SetType& Set)
	{
		for (auto& Element : Set)
		{
			return &Element;
		}

		return nullptr;
	}
	

	/**
	 * Tries to create the default graph for the Quest if the number of nodes differ from the quest data and the graph data
	 *
	 * @param Dialogue		The Dialogue we want to create the default graph for.
	 * @param bPrompt		Indicates if we should prompt the user for a response.
	 */
	static void TryToCreateDefaultGraph(UQuestAsset* Quest, bool bPrompt = true);

	// Get the DialogueEditor for given object, if it exists
	static TSharedPtr<class FQuestAssetEditorToolkit> GetQuestEditorForGraph(const UEdGraph* Graph);
	
};


