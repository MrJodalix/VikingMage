//TODO: Copyright
#pragma once

#include "IQuestEditorAccess.h"
#include "QuestEditorUtilities.h"

class QUESTASSETEDITOR_API FQuestEditorAccess : public IQuestEditorAccess
{
public:
	FQuestEditorAccess(){}
	~FQuestEditorAccess(){}

	virtual void UpdateGraphNodeEdges(UEdGraphNode* GraphNode) override;
	virtual UEdGraph* CreateNewQuestGraph(UQuestAsset* QuestAsset) const override;
	virtual void CompileQuestNodesFromGraphNodes(UQuestAsset* Quest) const override;
	virtual void RemoveAllGraphNodes(UQuestAsset* Dialogue) const override;
	virtual void UpdateQuestToVersion_UseOnlyOneOutputAndInputPin(UQuestAsset* Quest) const override;
	virtual void SetNewOuterForObjectFromGraphNode(UObject* Object, UEdGraphNode* GraphNode) const override;

	bool AreQuestNodesInSyncWithGraphNodes(UQuestAsset* QuestAsset) const override
	{
		return FQuestEditorUtilities::AreQuestNodesInSyncWithGraphNodes(QuestAsset);
	}
	

};