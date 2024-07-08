//TODO: Copyright
#pragma once

#include "EdGraph/EdGraphSchema.h"
#include "Templates/SubclassOf.h"

#include "QuestAsset/Private/Objective.h"

#include "NewNode_QuestGraphSchemaAction.generated.h"

class UQuestGraphNode;
class UQuestAsset;
class UEdGraph;

/** Action to add a node to the graph */
USTRUCT()
struct FNewNode_QuestGraphSchemaAction : public FEdGraphSchemaAction
{
private:
	typedef FNewNode_QuestGraphSchemaAction Self;

public:
	GENERATED_USTRUCT_BODY();

	FNewNode_QuestGraphSchemaAction() : FEdGraphSchemaAction(){}
	FNewNode_QuestGraphSchemaAction(
		const FText& InNodeCategory,
		const FText& InMenuDesc, const FText& InToolTip,
		int32 InGrouping, TSubclassOf<UObjective> InCreateNodeType
	) : FEdGraphSchemaAction(InNodeCategory, InMenuDesc, InToolTip, InGrouping), CreateNodeType(InCreateNodeType) {}


	/* ----- FEdGraphSchemaAction Interface ----- */
	UEdGraphNode* PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, bool bSelectNewNode) override;
	/* ***** FEdGraphSchemaAction Interface ***** */

	//Spawns a new UQuestGraphNode of type GraphNodeType that must have a valid Objective of TSubclassOf<UObjective>
	template <typename GraphNodeType>
	static GraphNodeType* SpawnGraphNodeWithObjectiveFromTemplate(
		UEdGraph* ParentGraph,
		TSubclassOf<UObjective> CreateNodeType,
		const FVector2D Location,
		bool bSelectedNewNode = true
	)
	{
		Self Action(FText::GetEmpty(), FText::GetEmpty(), FText::GetEmpty(), 0, CreateNodeType);
		return CastChecked<GraphNodeType>(Action.PerformAction(ParentGraph, nullptr, Location, bSelectedNewNode));
	}

private:

	/** Create a new quest node from the template */
	UEdGraphNode* CreateNode(UQuestAsset* QuestAsset, UEdGraph* ParentGraph, UEdGraphPin* FromPin, FVector2D Location, bool bSelectNewNode);

	/** The node type used for when creating a new node (used by CreateNode) */
	TSubclassOf<UObjective> CreateNodeType;	
};