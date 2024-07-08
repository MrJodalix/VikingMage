//TODO: Copyright
#pragma once

#include "EdGraph/EdGraphSchema.h"
#include "Templates/SubclassOf.h"

#include "NewComment_QuestGraphSchemaAction.generated.h"

class UEdGraph;

/** Action to create a new comment in the graph */
USTRUCT()
struct FNewComment_QuestGraphSchemaAction : public FEdGraphSchemaAction
{
	GENERATED_USTRUCT_BODY();

	FNewComment_QuestGraphSchemaAction() : FEdGraphSchemaAction() {}
	FNewComment_QuestGraphSchemaAction(const FText& InNodeCategory, const FText& InMenuDesc, const FText& InToolTip, int32 InGrouping)
		: FEdGraphSchemaAction(InNodeCategory, InMenuDesc, InToolTip, InGrouping) {}

	/* ---- FEdGraphSchemaAction Interface ----- */
	UEdGraphNode* PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, bool bSelectNewNode = true) override;
	/* ***** FEdGraphSchemaAction Interface ***** */

};