//TODO: Copyright
#include "QuestEditorAccess.h"

#include "QuestAsset.h"
#include "Toolkits/Graph/QuestGraph.h"
#include "Toolkits/Graph/QuestGraphSchema.h"
#include "QuestAssetCompiler.h"

void FQuestEditorAccess::UpdateGraphNodeEdges(UEdGraphNode* GraphNode)
{
	//TODO: Nötig? Nö!
}

UEdGraph* FQuestEditorAccess::CreateNewQuestGraph(UQuestAsset* QuestAsset) const
{
	UQuestGraph* QuestGraph = CastChecked<UQuestGraph>(FQuestEditorUtilities::CreateNewGraph(QuestAsset, NAME_None,
		UQuestGraph::StaticClass(), UQuestGraphSchema::StaticClass()));

	QuestGraph->bAllowDeletion = false;

	return CastChecked<UEdGraph>(QuestGraph);
}

void FQuestEditorAccess::CompileQuestNodesFromGraphNodes(UQuestAsset* Quest) const
{
	FCompilerResultsLog MessageLog;
	FQuestCompilerContext CompilerContext(Quest, MessageLog);
	CompilerContext.Compile();	
}


void FQuestEditorAccess::RemoveAllGraphNodes(UQuestAsset* Quest) const
{
	CastChecked<UQuestGraph>(Quest->GetGraph())->RemoveAllNodes();

	// Clear the references from the Dialogue Nodes
	//Quest->GetMutableStartNode()->ClearGraphNode();
	const TArray<UObjective*>& Nodes = Quest->GetObjectives();
	const int32 NodesNum = Nodes.Num();
	for (int32 NodeIndex = 0; NodeIndex < NodesNum; NodeIndex++)
	{
		Nodes[NodeIndex]->ClearGraphNode();
	}
}

void FQuestEditorAccess::UpdateQuestToVersion_UseOnlyOneOutputAndInputPin(UQuestAsset* Dialogue) const
{
	//TODO Testen: Nötig?
}

void FQuestEditorAccess::SetNewOuterForObjectFromGraphNode(UObject* Object, UEdGraphNode* GraphNode) const
{
	if (!Object || !GraphNode)
	{
		return;
	}

	UObjective* ClosestNode = FQuestEditorUtilities::GetClosestNodeFromGraphNode(GraphNode);
	if (!ClosestNode)
	{
		return;
	}

	Object->Rename(nullptr, ClosestNode, REN_DontCreateRedirectors);
}





