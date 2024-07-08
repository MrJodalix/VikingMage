//TODO: Copyright
#include "QuestAssetCompiler.h"

#include "QuestGraph.h"
#include "QuestEditorUtilities.h"
#include "Algo/Reverse.h"


void FQuestCompilerContext::Compile()
{
	check(Quest);
	UQuestGraph* QuestGraph = CastChecked<UQuestGraph>(Quest->GetGraph());
	QuestGraphNodes = QuestGraph->GetAllQuestGraphNodes();
	if (QuestGraphNodes.Num() == 0)
		return;

	ResultObjectives.Empty();
	VisitedNodes.Empty();
	Queue.Empty();
	IndicesHistory.Empty();
	NodesPath.Empty();
	//Start with one as idx 0 is always the root!
	NextAvailableIndex = 1;

	//Step 1: Find the root and set the start node
	GraphNodeRoot = QuestGraph->GetRootGraphNode();
	check(GraphNodeRoot);
	VisitedNodes.Add(GraphNodeRoot);
	//TODO (Testen): So richtig? An idx 0 soll immer das RootObjective stehen
	ResultObjectives.Add(GraphNodeRoot->GetMutableQuestNode());

	NodeDepth = 0;
	NodesNumberUntilDepthIncrease = 1; // root/start node
	NodesNumberNextDepth = 0; // we do not know yet

	CompileGraphNode(GraphNodeRoot);

	// Step 2. Walk the graph and set the rest of the nodes.
	CompileGraph();

	// Step 3. Add orphan nodes (nodes / node group with no parents), not connected to the start node
	PruneIsolatedNodes();

	// Step 4. Update the dialogue data.
	Quest->EmptyNodesGUIDToIndexMap();
	Quest->SetNodes(ResultObjectives);

	// Step 5. Fix old indices and update GUID for the Conditions
	FixBrokenOldIndicesAndUpdateGUID();

	Quest->PostEditChange();
}

void FQuestCompilerContext::PreCompileGraphNode(UQuestGraphNode* GraphNode)
{
	//Update objective Children! Remove all child objectives. Then get all objectives of child
	//nodes and set them as the new children
	GraphNode->GetMutableQuestNode()->RemoveAllChildren();
	TArray<UObjective*> GraphNodeChildren;

	for(UQuestGraphNode*& Node : GraphNode->GetChildNodes())
	{
		GraphNodeChildren.Add(Node->GetMutableQuestNode());
	}
	GraphNode->GetMutableQuestNode()->SetNodeChildren(GraphNodeChildren);

	// Do the same for the parent nodes
	GraphNode->GetMutableQuestNode()->RemoveAllParents();
	TArray<UObjective*> GraphNodeParents;

	for (UQuestGraphNode*& Node : GraphNode->GetParentNodes())
	{
		GraphNodeParents.Add(Node->GetMutableQuestNode());
	}
	GraphNode->GetMutableQuestNode()->SetNodeParents(GraphNodeParents);

	
	// Sort children and parents so that they're organized the same as user can see in the editor.		
	GraphNode->SortChildrenBasedOnYLocation();
	GraphNode->SortParentsBasedOnYLocation();
	GraphNode->CheckQuestNodeSyncWithGraphNode();
	GraphNode->SetNodeDepth(NodeDepth);
	NodeUnvisitedChildrenNum = 0;
}

void FQuestCompilerContext::PostCompileGraphNode(UQuestGraphNode* GraphNode)
{
	GraphNode->ApplyCompilerWarnings();

	// Check symmetry, dialogue node <-> graph node
	GraphNode->CheckQuestNodeSyncWithGraphNode(true);

	// Ensure the Node has a valid GUID
	UObjective* QuestNode = GraphNode->GetMutableQuestNode();
	if (!QuestNode->HasGUID())
	{
		QuestNode->RegenerateGUID();
	}

	// Update depth
	// BFS has the property that unvisited nodes in the queue all have depths that never decrease,
	// and increase by at most 1.
	--NodesNumberUntilDepthIncrease;

	// Track NodeDepth + 1 number
	NodesNumberNextDepth += NodeUnvisitedChildrenNum;
	if (NodesNumberUntilDepthIncrease == 0)
	{
		// Next depth coming up!
		NodeDepth++;
		NodesNumberUntilDepthIncrease = NodesNumberNextDepth;
		// Reset for the next dept aka NodeDepth + 1
		NodesNumberNextDepth = 0;
	}
}

void FQuestCompilerContext::CompileGraphNode(UQuestGraphNode* GraphNode)
{
	PreCompileGraphNode(GraphNode);

	// Get the data as mutable, so what we can modify inplace
	const UObjective& QuestNode = GraphNode->GetQuestNode();
	const TArray<UObjective*>& NodeChildren = QuestNode.GetNodeChildren();;

	// Walk over direct children
	// NOTE: GraphNode.OutputPin.LinkedTo are kept in sync with the Quest.Objectives
	//const TArray<UObjective*>& QuestObjectives = Quest->GetObjectives();
	const TArray<UQuestGraphNode*> ChildNodes = GraphNode->GetChildNodes();

	for (int32 ChildIndex = 0, ChildrenNum = ChildNodes.Num(); ChildIndex < ChildrenNum; ChildIndex++)
	{
		//TODO (Checked): Prüfen, ob ChildObjectives valid sind

		// Only Subclasses of UObjective should be valid!
		check(NodeChildren[ChildIndex]->IsValid())
			//the current child node
			UQuestGraphNode* ChildNode = ChildNodes[ChildIndex];

		// Sanity check to assume that the child node will have the same edge data from the parent
		// BEFORE TargetIndex reassignment. If this fails it means that the Dialogue Node Children are not in
		// the right order (assumption below fails).
		//const int32 ChildNodeTargetIndex = NodeChildren[ChildIndex].TargetIndex;
		//check(ChildNode == QuestObjectives[ChildNodeTargetIndex]->GetGraphNode())

			// Prevent double visiting nodes
		if (VisitedNodes.Contains(ChildNode))
		{
			// Node already visited, we don't have to consider it more than once
			//GraphNode->SetEdgeTargetIndexAt(ChildIndex, ChildNode->GetDialogueNodeIndex());
			continue;
		}

		// Traverse the queue
		verify(Queue.Enqueue(ChildNode));
		VisitedNodes.Add(ChildNode);

		// From This Node => Parent Node
		NodesPath.Add(ChildNode, GraphNode);
		NodeUnvisitedChildrenNum++;

		// Assume they will be added in order.
		// Parent (GraphNode) points to the new assigned ChildNode.NodeIndex
		SetNextAvailableIndexToNode(ChildNode);
		//GraphNode->SetEdgeTargetIndexAt(ChildIndex, NextAvailableIndex);
		NextAvailableIndex++;
	}

	// Update the graph node/dialogue node with the new Node data (indices of children)
	PostCompileGraphNode(GraphNode);
}

void FQuestCompilerContext::CompileGraph()
{
	// Complexity O(|V| + |E|)
	// Reassign all the indices in the queue
	while (!Queue.IsEmpty())
	{
		//Gets last element from queue 
		UQuestGraphNode* GraphNode;
		verify(Queue.Dequeue(GraphNode));

		// See children
		CompileGraphNode(GraphNode);

		// Accumulate for the Dialogue.Nodes array
		ResultObjectives.Add(GraphNode->GetMutableQuestNode());

		// Expect prediction to be true
		//check(NodeAddedIndex == GraphNode->GetDialogueNodeIndex());
	}
}


void FQuestCompilerContext::PruneIsolatedNodes()
{
	//checks if all nodes in the whole graph have been visited
	if (QuestGraphNodes.Num() == VisitedNodes.Num())
	{
		return;
	}

	const TSet<UQuestGraphNode*> NodesSet(QuestGraphNodes);
	// Get every orphan
	while (NodesSet.Num() != VisitedNodes.Num())
	{
		// Nodes that are in the graph but not in the visited nodes set
		const TSet<UQuestGraphNode*> OrphanedNodes = NodesSet.Difference(VisitedNodes);

		// Try to find the root orphan (the one with 0 inputs)
		// This will fail  if the orphan subgraph is cyclic.
		UQuestGraphNode* RootOrphan = nullptr;
		for (UQuestGraphNode* GraphNode : OrphanedNodes)
		{
			if (GraphNode->GetInputPin()->LinkedTo.Num() == 0)
			{
				RootOrphan = GraphNode;
				break;
			}
		}
		// Cyclic orphan subgraph found, choose first node (RootOrphan still null)
		if (!IsValid(RootOrphan))
		{
			RootOrphan = CastChecked<UQuestGraphNode>(*FQuestEditorUtilities::GetFirstSetElement(OrphanedNodes));
		}

		// Queue and assign node
		SetNextAvailableIndexToNode(RootOrphan);
		VisitedNodes.Add(RootOrphan);
		Queue.Empty();
		verify(Queue.Enqueue(RootOrphan));
		NextAvailableIndex++;
		CompileGraph();
	}
}

void FQuestCompilerContext::FixBrokenOldIndicesAndUpdateGUID()
{
	//TODO (Testen): nötig?
	return;
}

void FQuestCompilerContext::SetNextAvailableIndexToNode(UQuestGraphNode* GraphNode)
{
	//TODO: Wo ist der Index der Nodes? Den müsste ich hier setzen, damit für jeden Node die
	// Position in den Objectives bekannt ist!
	GraphNode->SetQuestNodeIndex(NextAvailableIndex);
}







