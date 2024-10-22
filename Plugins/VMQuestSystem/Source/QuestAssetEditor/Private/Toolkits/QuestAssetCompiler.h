//TODO: Copyright
#pragma once

#include "CoreTypes.h"
#include "Kismet2/CompilerResultsLog.h"
#include "Containers/Queue.h"

class UObjective;
class UQuestAsset;
class UQuestGraphNode;

class FQuestCompilerContext
{
	//friend grants UQuestAsset access to private and protected members of this class
	friend UQuestAsset;

public:
	//TODO (Testen): Brauchen wir die Settings?
	FQuestCompilerContext(UQuestAsset* InQuest, FCompilerResultsLog& InMessageLog)
		: Quest(InQuest), MessageLog(InMessageLog)
	{}

	/** Compile the Quest from its graph nodes, that means converting the
	 * information given from the graph to actual objectives in an array */
	void Compile();

private:
	/** Applies necessary steps before the main compile routine (CompileGraphNode) has compiled on a node. */
	void PreCompileGraphNode(UQuestGraphNode* GraphNode);

	/** Applies necessary steps after the main compile routine (CompileGraphNode) has finished on a node. */
	void PostCompileGraphNode(UQuestGraphNode* GraphNode);

	/** Save direct children of node in queue, if not already visited. Order in Queue represents the new array order
	 * in the end */
	void CompileGraphNode(UQuestGraphNode* GraphNode);

	/** Used for the step 2 and 4 of the compile phase, reassign/compile the whole graph queue. */
	void CompileGraph();

	/** Gets the Path from SourceNode to the TargetNode in the OutPath. Returns false if no path can be found.  */
	bool GetPathToNode(const UQuestGraphNode* SourceNode, const UQuestGraphNode* TargetNode, TArray<const UQuestGraphNode*>& OutPath);

	/** Same as above only return a set. */
	bool GetPathToNodeAsSet(const UQuestGraphNode* SourceNode, const UQuestGraphNode* TargetNode, TSet<const UQuestGraphNode*>& OutNodesInPath);

	/** Compiles/handles all remaining isolated nodes of the graph. */
	void PruneIsolatedNodes();

	/** Fixes all references to the old indices that this compile most likely broke. */
	void FixBrokenOldIndicesAndUpdateGUID();

	/** Sets NextAvailableIndex on the provided nodes, also keeps track of history in IndicesHistory. */
	void SetNextAvailableIndexToNode(UQuestGraphNode* GraphNode);

private:
	UQuestAsset* Quest = nullptr;

	/** Compiler message log (errors, warnings, notes) */
	FCompilerResultsLog& MessageLog;

	// Used when compiling:
	/** All the graph nodes of the Quest */
	TArray<UQuestGraphNode*> QuestGraphNodes;

	/** The result of the compile will be stored here. This represents the new QuestAsset.Objectives Array. */
	TArray<UObjective*> ResultObjectives;

	/** All the visited graph nodes in the BFS. */
	TSet<UQuestGraphNode*> VisitedNodes;

	/** The queue for the BFS */
	TQueue<UQuestGraphNode*> Queue;

	/**
	 * Used to keep track of how indices change.
	 * Key: Old index
	 * Value: New index
	 */
	TMap<int32, int32> IndicesHistory;

	/** The root graph node. */
	UQuestGraphNode* GraphNodeRoot = nullptr;

	/**
	 * Keep track of paths. Use pointers as the indices change at compile time.
	 * Key: Node
	 * Value: Parent Node
	 */
	TMap<const UQuestGraphNode*, const UQuestGraphNode*> NodesPath;

	/** Useful for keeping track of the next available index in the DialogueNodes. */
	int32 NextAvailableIndex = INDEX_NONE;

	/** Indicates the current node depth the BFS is at. Root Node as depth 0. */
	int32 NodeDepth = INDEX_NONE;

	/** Tells us the number of unvisited children the current node has. */
	int32 NodeUnvisitedChildrenNum = INDEX_NONE;

	/**
	 * Keeps track of how many nodes we must process (take from the queue) until we reach the next depth.
	 * When it reaches 0 it means we have reached the next level.
	 */
	int32 NodesNumberUntilDepthIncrease = INDEX_NONE;

	/**
	 * Keeps track of the nodes numbers at depth NodeDepth + 1.
	 * Useful because we are only looking at the current depth and Unreal Queue does not have Size() sigh.
	 * This could have been done with a bool only, because we know how many items there are at current level and when
	 * we take all of them out, we know that the size of the current Queue is the nodes number at the next depth.
	 */
	int32 NodesNumberNextDepth = INDEX_NONE;

};