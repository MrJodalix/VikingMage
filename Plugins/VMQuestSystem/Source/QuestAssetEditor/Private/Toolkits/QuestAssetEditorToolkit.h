// TODO: Copyright

#pragma once

#include "EditorUndoClient.h"
#include "QuestGraph.h"
#include "QuestAsset/Private/Objective.h"
#include "QuestGraphNode.h"

#include "Templates/SharedPointer.h"
#include "Toolkits/AssetEditorToolkit.h"
#include "UObject/GCObject.h"

class FSpawnTabArgs;
class IToolkitHost;
class SDockTab;
class UQuestAsset;

class FQuestAssetEditorToolkit
	: public FAssetEditorToolkit
	//Interface for Undoing Actions
	, FEditorUndoClient
	, public FGCObject
	, public FNotifyHook
{
	typedef FQuestAssetEditorToolkit Self;

public:

	/* ----- Constructor and Deconstructor ----- */

	/**
	 * Constructor, creates and initializes a new instance.
	 */
	FQuestAssetEditorToolkit();

	/** Virtual destructor. */
	virtual ~FQuestAssetEditorToolkit();

public:
	/// <summary>
	/// Initializes the editor tool kit.
	/// </summary>
	/// <param name="InQuestAsset">The UQuestAsset to edit.</param>
	/// <param name="InMode">The mode to create the toolkit in.</param>
	/// <param name="InToolkitHost">The toolkit host.</param>
	void Initialize(UQuestAsset* InQuestAsset, const EToolkitMode::Type InMode, const TSharedPtr<IToolkitHost>& InToolkitHost);


public:
	/* ----- FAssetEditorToolkit interface ----- */

	/// <summary>
	/// Gets a link to the documentation.
	/// </summary>
	/// <returns>Documentation link</returns>
	virtual FString GetDocumentationLink() const override;
	virtual void RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager) override;
	virtual void UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager) override;

public:
	/* ----- IToolkit interface ---- */
	virtual FText GetBaseToolkitName() const override;
	virtual FName GetToolkitFName() const override;
	virtual FLinearColor GetWorldCentricTabColorScale() const override;
	virtual FString GetWorldCentricTabPrefix() const override;

public:
	/* ----- FGCObject interface ----- */

	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;

	/* ---- Own Functions ----- */

	//Get the currently selected set of nodes
	TSet<UObject*> GetSelectedNodes() const
	{
		check(GraphEditorView.IsValid());
		return GraphEditorView->GetSelectedNodes();
	}


	/**
	 * Get the bounding area for the currently selected nodes
	 *
	 * @param Rect Final output bounding area, including padding
	 * @param Padding An amount of padding to add to all sides of the bounds
	 *
	 * @return false if nothing is selected
	 */
	bool GetBoundsForSelectedNodes(class FSlateRect& Rect, float Padding) const
	{
		return GraphEditorView->GetBoundsForSelectedNodes(Rect, Padding);
	}

	// Clears the viewport selection set
	void ClearViewportSelection() const
	{
		if (GraphEditorView.IsValid())
		{
			GraphEditorView->ClearSelectionSet();
		}
	}

	// Refreshes the graph viewport.
	void RefreshViewport() const
	{
		UE_LOG(LogTemp, Display, TEXT("Should Refresh Graph"));
		if (GraphEditorView.IsValid())
		{
			UE_LOG(LogTemp, Display, TEXT("Refreshed Graph"));
			GraphEditorView->NotifyGraphChanged();			
		}
	}

	// Refreshes the details panels for the quest and the objectives
	void RefreshDetailsView(bool bRestorePreviousSelection);
	void Refresh(bool bRestorePreviousSelection);
	
	// Helper method to get directly the Dialogue Graph
	UQuestGraph* GetQuestGraph() const { return CastChecked<UQuestGraph>(CurrentQuestAsset->GetGraph()); }

	// Performs checks on all the nodes in the Dialogue
	void CheckAll() const;


	int GetPreviousSelectedNodesAmount() const
	{
		return PreviousSelectedNodeObjects.Num();
	}

protected:
	/* ----- FEditorUndoClient Interface ----- */

	virtual void PostUndo(bool bSuccess) override;
	virtual void PostRedo(bool bSuccess) override;

private:
	/* -- Tab Creation --*/
	/** Callback for spawning the Properties tab. */
	TSharedRef<SDockTab> SpawnTab_QuestDetails(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> SpawnTab_GraphCanvas(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> SpawnTab_ObjectiveDetails(const FSpawnTabArgs& Args);

	/* -- Widget Creation --*/
	/**Create all Widgets */
	void CreateInternalWidgets();
	/**Create Quest Details Widget*/
	void CreateQuestDetailsWidget();
	/** Create a Graph Widget to design the Quest */
	TSharedRef<SGraphEditor> CreateGraphEditorWidget();
	void CreateObjectiveDetailsWidget();

	// Bind the commands from the editor. See GraphEditorCommands for more details.
	void BindEditorCommands();

	/* -- Node Commands -- */
	//
	// Edit Node commands
	//

	// Converts an objective to another objective!	
	void OnCommandConvertObjectiveToKill() const;
	void OnCommandConvertObjectiveToLocate() const;
	void OnCommandConvertObjectiveToCollect() const;
	void OnCommandConvertObjectiveToInteract() const;

	// Remove the currently selected nodes from editor view
	void OnCommandDeleteSelectedNodes() const;

	// Whether we are able to remove the currently selected nodes
	bool CanDeleteNodes() const;

	// Copy the currently selected nodes to the text buffer.
	void OnCommandCopySelectedNodes() const;

	// Whether we are able to copy the currently selected nodes.
	bool CanCopyNodes() const;

	// Paste the nodes at the current location
	void OnCommandPasteNodes();

	// Paste the nodes at the specified Location.
	void PasteNodesHere(const FVector2D& Location);

	// Whether we are able to paste from the clipboard
	bool CanPasteNodes() const;


	/* -- Graph Events -- */

	/** Called when the selection changes in the GraphEditor */
	void OnSelectedNodesChanged(const TSet<UObject*>& NewSelection);

	/**Called  to create context menu when right-clicking on graph*/
	FActionMenuContent OnCreateGraphActionMenu(UEdGraph* InGraph,
		const FVector2D& InNodePosition,
		const TArray<UEdGraphPin*>& InDraggedPins,
		bool bAutoExpand,
		SGraphEditor::FActionMenuClosed InOnMenuClosed);

	/** Called from graph context menus when they close to tell the editor why they closed */
	void OnGraphActionMenuClosed(bool bActionExecuted, bool bGraphPinContext);

	/**
	 * Called when a node's title is committed for a rename
	 *
	 * @param NewText			New title text
	 * @param CommitInfo		How text was committed
	 * @param NodeBeingChanged	The node being changed
	 */
	void OnNodeTitleCommitted(const FText& NewText, ETextCommit::Type CommitInfo, UEdGraphNode* NodeBeingChanged) const;

	template<typename T>
	void ConvertNodes() const
	{
		TSet<UObject*> SelectedNodes = GetSelectedNodes();

		for (UObject* NodeObject : SelectedNodes)
		{
			UQuestGraphNode* ConvertNode = Cast<UQuestGraphNode>(NodeObject);
			T* CheckOldObjective = Cast<T>(ConvertNode->GetMutableQuestNode());
			//Only Convert QuestGraphNodes
			//TODO: Prohibit converting to the same objective again
			if (ConvertNode && !CheckOldObjective)
			{
				UObjective* OldObjective = ConvertNode->GetMutableQuestNode();								
				T* NewObjective = CurrentQuestAsset->ConstructQuestNode<T>();
				NewObjective->SetNodeChildren(OldObjective->GetNodeChildren());
				//NewObjective->SetGraphNode(OldObjective->GetGraphNode());				
				NewObjective->RegenerateGUID();
				NewObjective->Description = OldObjective->Description;
				NewObjective->IsQuestEnding = OldObjective->IsQuestEnding;
				NewObjective->State = OldObjective->State;

				//Set new Quest Node
				ConvertNode->SetQuestNode(NewObjective);
			}

		}
	}
	/* -- Own Methods --*/


private:
	/**
	 * Current Quest being edited.
	 */
	UQuestAsset* CurrentQuestAsset;

	/** Current Objective (Node) selected */
	//UObjective* CurrentObjective;

	// Keep track of the previous selected objects so that we can reverse selection
	TArray<TWeakObjectPtr<UObject>> PreviousSelectedNodeObjects;

	// Command list for this editor. Synced with FQuestEditorCommands. Aka list of shortcuts supported.
	TSharedPtr<FUICommandList> GraphEditorCommands;

	/** Graph Editor */
	TSharedPtr<SGraphEditor> GraphEditorView;
	/** Custom Details view for quests info */
	TSharedPtr<IDetailsView> QuestDetailsView;
	/**Custom Details view for each objective, when selected*/
	TSharedPtr<IDetailsView> ObjectiveDetailsView;


};