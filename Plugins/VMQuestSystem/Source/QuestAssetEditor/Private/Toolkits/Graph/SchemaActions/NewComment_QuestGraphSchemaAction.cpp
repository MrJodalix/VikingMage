//TODO: Copyright
#include "NewComment_QuestGraphSchemaAction.h"

#include "EdGraphNode_Comment.h"

#include "QuestEditorUtilities.h"

#define LOCTEXT_NAMESPACE "NewComment_QuestGraphSchemaAction"

UEdGraphNode* FNewComment_QuestGraphSchemaAction::PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin, 
	const FVector2D Location, bool bSelectNewNode) 
{
	//Add menu item for creating comment boxes
	UEdGraphNode_Comment* CommentTemplate = NewObject<UEdGraphNode_Comment>();

	//Wrap comment around other nodes, this makes it possible to select other nodes and press the "c" key on the keyboard
	FVector2D SpawnLocation = Location;
	FSlateRect Bounds;
	if(FQuestEditorUtilities::GetBoundsForSelectedNodes(ParentGraph, Bounds, 50.0f))
	{
		CommentTemplate->SetBounds(Bounds);
		SpawnLocation.X = CommentTemplate->NodePosX;
		SpawnLocation.Y = CommentTemplate->NodePosY;
	}

	return FEdGraphSchemaAction_NewNode::SpawnNodeFromTemplate<UEdGraphNode_Comment>(ParentGraph, CommentTemplate, SpawnLocation, bSelectNewNode);
}

#undef LOCTEXT_NAMESPACE
