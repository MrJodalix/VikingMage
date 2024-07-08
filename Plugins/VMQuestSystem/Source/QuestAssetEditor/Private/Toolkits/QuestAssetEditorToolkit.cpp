//TODO: Copyright

#include "QuestAssetEditorToolkit.h"

#include "Editor.h"
#include "EditorReimportHandler.h"
#include "EditorStyleSet.h"
#include "GraphEditorActions.h"
#include "UObject/NameTypes.h"
#include "Widgets/Docking/SDockTab.h"
#include "Framework/Commands/GenericCommands.h"
#include "EdGraphUtilities.h"
#include "HAL/PlatformApplicationMisc.h"

#include "QuestAsset.h"
#include "QuestEditorUtilities.h"
#include "SQuestActionMenu.h"
#include "QuestCommands.h"
#include "QuestAsset/Private/KillObjective.h"
#include "QuestAsset/Private/LocationObjective.h"
#include "QuestAsset/Private/CollectObjective.h"
#include "QuestAsset/Private/InteractObjective.h"

#include "QuestAssetEditor/Private/Toolkits/Graph/SchemaActions/NewComment_QuestGraphSchemaAction.h"


#define LOCTEXT_NAMESPACE "FQuestAssetEditorToolkit"

DEFINE_LOG_CATEGORY_STATIC(LogQuestAssetEditor, Log, All);

/* -----  Local constants ----- */

//Constants for identifying the quest asset editor toolkit and tab
namespace QuestAssetEditor
{
	static const FName AppIdentifier("QuestAssetEditorApp");
	static const FName GraphCanvasID("QuestEditor");
	static const FName QuestDetailsTabID("QuestDetails");
	static const FName ObjectDetailsTabID("ObjectDetails");
}

/* ----- Constructor and destructor ----- */

FQuestAssetEditorToolkit::FQuestAssetEditorToolkit()
	: CurrentQuestAsset(nullptr)
{ }

FQuestAssetEditorToolkit::~FQuestAssetEditorToolkit()
{
	FReimportManager::Instance()->OnPreReimport().RemoveAll(this);
	FReimportManager::Instance()->OnPostReimport().RemoveAll(this);

	GEditor->UnregisterForUndo(this);
}

/* ----- FQuestAssetEditorToolkit interface ----- */

void FQuestAssetEditorToolkit::Initialize(UQuestAsset* InQuestAsset, const EToolkitMode::Type InMode, const TSharedPtr<IToolkitHost>& InToolkitHost)
{
	//TODO: close all other editors editing this asset

	CurrentQuestAsset = InQuestAsset;
	FQuestEditorUtilities::TryToCreateDefaultGraph(CurrentQuestAsset);

	//Support undo/redo
	CurrentQuestAsset->SetFlags(RF_Transactional);
	GEditor->RegisterForUndo(this);

	//Bind commands
	FGraphEditorCommands::Register();
	FQuestCommands::Register();
	BindEditorCommands();
	CreateInternalWidgets();


	//create tab layout		
	// Default layout
	const TSharedRef<FTabManager::FLayout> StandaloneDefaultLayout =
		FTabManager::NewLayout("Standalone_QuestEditor_Layout_v0.0.1")
		->AddArea
		(
			FTabManager::NewPrimaryArea()
			->SetOrientation(Orient_Vertical)
			->Split
			(
				// Toolbar
				FTabManager::NewStack()
				->SetSizeCoefficient(0.1f)
				->AddTab(GetToolbarTabId(), ETabState::OpenedTab)
				->SetHideTabWell(true)
			)
			->Split
			(
				// Main Application area
				FTabManager::NewSplitter()
				->SetOrientation(Orient_Horizontal)
				->SetSizeCoefficient(0.9f)
				->Split
				(
					// Left
					// Details tab
					FTabManager::NewStack()
					->SetSizeCoefficient(0.2f)
					->AddTab(QuestAssetEditor::QuestDetailsTabID, ETabState::OpenedTab)
				)
				->Split
				(

					// Top
					// Graph canvas
					FTabManager::NewStack()
					->SetSizeCoefficient(0.6f)
					->SetHideTabWell(true)
					->AddTab(QuestAssetEditor::GraphCanvasID, ETabState::OpenedTab)

				)
				->Split
				(
					// Right
					// Properties tab
					FTabManager::NewStack()
					->SetSizeCoefficient(0.2f)
					->AddTab(QuestAssetEditor::ObjectDetailsTabID, ETabState::OpenedTab)
				)

			)
		);

	FAssetEditorToolkit::InitAssetEditor(
		InMode,
		InToolkitHost,
		QuestAssetEditor::AppIdentifier,
		StandaloneDefaultLayout,
		true /*bCreateDefaultStandaloneMenu*/,
		true /*bCreateDefaultToolbar*/,
		InQuestAsset
	);

	RegenerateMenusAndToolbars();
}

//***********************************************************************
/* ----- FAssetEditorToolkit interface ----- */
FString FQuestAssetEditorToolkit::GetDocumentationLink() const
{
	return FString(TEXT("https://github.com/Fritte795/VikingMage/wiki/VMQuestSystem"));
}

void FQuestAssetEditorToolkit::RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	WorkspaceMenuCategory = InTabManager->AddLocalWorkspaceMenuCategory(LOCTEXT("WorkspaceMenu_QuestAssetEditor", "Quest Asset Editor"));
	const auto WorkspaceMenuCategoryRef = WorkspaceMenuCategory.ToSharedRef();

	FAssetEditorToolkit::RegisterTabSpawners(InTabManager);

	//Spawn tabs
	InTabManager->RegisterTabSpawner(
		QuestAssetEditor::GraphCanvasID,
		FOnSpawnTab::CreateSP(this, &FQuestAssetEditorToolkit::SpawnTab_GraphCanvas))
		.SetDisplayName(LOCTEXT("QuestEditorTabName", "Viewport"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "GraphEditor.EventGraph_16x"));

	InTabManager->RegisterTabSpawner(
		QuestAssetEditor::QuestDetailsTabID,
		FOnSpawnTab::CreateSP(this, &FQuestAssetEditorToolkit::SpawnTab_QuestDetails))
		.SetDisplayName(LOCTEXT("QuestDetailsTabLabel", "Quest Details"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.Tabs.Details"));

	InTabManager->RegisterTabSpawner(
		QuestAssetEditor::ObjectDetailsTabID,
		FOnSpawnTab::CreateSP(this, &FQuestAssetEditorToolkit::SpawnTab_ObjectiveDetails))
		.SetDisplayName(LOCTEXT("ObjectDetailsTabLabel", "Object Details"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.Tabs.Details"));

}

void FQuestAssetEditorToolkit::UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	FAssetEditorToolkit::UnregisterTabSpawners(InTabManager);
	InTabManager->UnregisterTabSpawner(QuestAssetEditor::GraphCanvasID);
	InTabManager->UnregisterTabSpawner(QuestAssetEditor::QuestDetailsTabID);
	InTabManager->UnregisterTabSpawner(QuestAssetEditor::ObjectDetailsTabID);
}

/* ----- IToolkit interface ----- */

FText FQuestAssetEditorToolkit::GetBaseToolkitName() const
{
	const bool bDirtyState = CurrentQuestAsset->GetOutermost()->IsDirty();

	FFormatNamedArguments Args;
	Args.Add(TEXT("QuestName"), FText::FromString(CurrentQuestAsset->GetName()));
	Args.Add(TEXT("DirtyState"), bDirtyState ? FText::FromString(TEXT("*")) : FText::GetEmpty());
	return FText::Format(LOCTEXT("QuestEditorToolkitName", "{QuestName}{DirtyState}"), Args);
}

FName FQuestAssetEditorToolkit::GetToolkitFName() const
{
	return FName("QuestAssetEditor");
}

FLinearColor FQuestAssetEditorToolkit::GetWorldCentricTabColorScale() const
{
	return FLinearColor(0.3f, 0.2f, 0.5f, 0.5f);
}

FString FQuestAssetEditorToolkit::GetWorldCentricTabPrefix() const
{
	return LOCTEXT("WorldCentricTabPrefix", "QuestAsset ").ToString();
}

/* ----- End IToolkit interface ----- */
//***********************************************************************

/* ----- FGCObject interface ----- */

void FQuestAssetEditorToolkit::AddReferencedObjects(FReferenceCollector& Collector)
{
	Collector.AddReferencedObject(CurrentQuestAsset);
}

/* ----- FEditorUndoClient interface -----*/

void FQuestAssetEditorToolkit::PostUndo(bool bSuccess)
{
	//TODO: implement if needed
}


void FQuestAssetEditorToolkit::PostRedo(bool bSuccess)
{
	//TODO: implement if needed
}


/* ----- Own Methods ----- */

void FQuestAssetEditorToolkit::RefreshDetailsView(bool bRestorePreviousSelection)
{
	if (QuestDetailsView.IsValid())
	{
		if (CurrentQuestAsset)
		{
			QuestDetailsView->SetObject(CurrentQuestAsset, true);
		}
		else
		{
			QuestDetailsView->ForceRefresh();
		}
		QuestDetailsView->ClearSearch();
	}

	if (ObjectiveDetailsView.IsValid())
	{
		if (PreviousSelectedNodeObjects.Num() > 0)
		{
			ObjectiveDetailsView->SetObjects(PreviousSelectedNodeObjects, true);
		}
		else
		{
			ObjectiveDetailsView->ForceRefresh();
		}
		ObjectiveDetailsView->ClearSearch();
	}

	if (bRestorePreviousSelection)
	{
		UE_LOG(LogTemp, Display,
			TEXT("Refresh Details View - Try to Restor Selection, Amount: %d"), PreviousSelectedNodeObjects.Num());

		// Create local copy because this can be changed by node selection again
		TArray<TWeakObjectPtr<UObject>> ArrayCopy = PreviousSelectedNodeObjects;

		// Select all previous nodes
		for (const TWeakObjectPtr<UObject>& WeakObj : ArrayCopy)
		{
			if (!WeakObj.IsValid(false))
			{
				continue;
			}

			UObject* Object = WeakObj.Get();
			if (!IsValid(Object))
			{
				continue;
			}

			UEdGraphNode* GraphNode = Cast<UEdGraphNode>(Object);
			if (!IsValid(GraphNode))
			{
				continue;
			}

			GraphEditorView->SetNodeSelection(const_cast<UEdGraphNode*>(GraphNode), true);
		}
	}



}


void FQuestAssetEditorToolkit::Refresh(bool bRestorePreviousSelection)
{
	ClearViewportSelection();
	RefreshViewport();
	RefreshDetailsView(bRestorePreviousSelection);
	FSlateApplication::Get().DismissAllMenus();
}



void FQuestAssetEditorToolkit::CheckAll() const
{
#if DO_CHECK
	check(CurrentQuestAsset);
	UQuestGraph* Graph = GetQuestGraph();
	for (UEdGraphNode* GraphNode : Graph->Nodes)
	{
		check(GraphNode);
	}

	check(FQuestEditorUtilities::AreQuestNodesInSyncWithGraphNodes(CurrentQuestAsset));
	for (UQuestGraphNode* GraphNode : Graph->GetAllQuestGraphNodes())
	{
		GraphNode->CheckAll();
	}
#endif
}


//TODO: RefreshDetailsView

//TODO: Refresh

//TODO: Jump to Node

/* ***** END Own Methods ***** */

/* ----- FQuestAssetEditorToolkit callbacks ------ */

/* -- Tab Generation --*/

TSharedRef<SDockTab> FQuestAssetEditorToolkit::SpawnTab_QuestDetails(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId() == QuestAssetEditor::QuestDetailsTabID);
	return SNew(SDockTab)
		.Icon(FEditorStyle::GetBrush("GenericEditor.Tabs.Properties"))
		.Label(LOCTEXT("QuestDetailsTitle", "Quest Details"))
		[
			QuestDetailsView.ToSharedRef()
		];
}

TSharedRef<SDockTab> FQuestAssetEditorToolkit::SpawnTab_GraphCanvas(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId() == QuestAssetEditor::GraphCanvasID);
	return SNew(SDockTab)
		.Label(LOCTEXT("QuestGraphCanvasTitle", "Viewport"))
		[
			GraphEditorView.ToSharedRef()
		];
}

TSharedRef<SDockTab> FQuestAssetEditorToolkit::SpawnTab_ObjectiveDetails(const FSpawnTabArgs& Args)
{

	check(Args.GetTabId() == QuestAssetEditor::ObjectDetailsTabID);
	return SNew(SDockTab)
		.Icon(FEditorStyle::GetBrush("GenericEditor.Tabs.Properties"))
		.Label(LOCTEXT("ObjectiveDetailsTitle", "Objective Details"))
		[
			ObjectiveDetailsView.ToSharedRef()
		];
}

/* -- Widget Generation --*/

void FQuestAssetEditorToolkit::CreateInternalWidgets()
{
	// Graph Viewport
	GraphEditorView = CreateGraphEditorWidget();

	// Details View Quest
	CreateQuestDetailsWidget();

	//Details View Objective
	CreateObjectiveDetailsWidget();

}

void FQuestAssetEditorToolkit::CreateQuestDetailsWidget()
{
	FDetailsViewArgs DetailsViewArgs;
	DetailsViewArgs.bUpdatesFromSelection = false;
	DetailsViewArgs.bLockable = false;
	DetailsViewArgs.bAllowSearch = true;
	DetailsViewArgs.NotifyHook = this;
	DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::ObjectsUseNameArea;
	DetailsViewArgs.bHideSelectionTip = false;

	FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	QuestDetailsView = PropertyEditorModule.CreateDetailView(DetailsViewArgs);
	QuestDetailsView->SetObject(CurrentQuestAsset);
}

TSharedRef<SGraphEditor> FQuestAssetEditorToolkit::CreateGraphEditorWidget()
{
	//check() is like assert() in C
	check(CurrentQuestAsset);

	//Customize the appearance of the graph.
	FGraphAppearanceInfo AppearanceInfo;
	//Text appears on bottom right corner in the graph view.
	AppearanceInfo.CornerText = LOCTEXT("AppearanceCornerText_QuestSystem", "QUEST");
	//AppearanceInfo.InstructionText = LOCTEXT("AppearanceCornerText_QuestSystem", "Right Click to add new nodes.");

	//Bind graph events actions from the editor	
	SGraphEditor::FGraphEditorEvents InEvents;
	InEvents.OnTextCommitted = FOnNodeTextCommitted::CreateSP(this, &FQuestAssetEditorToolkit::OnNodeTitleCommitted);
	InEvents.OnSelectionChanged = SGraphEditor::FOnSelectionChanged::CreateSP(this, &FQuestAssetEditorToolkit::OnSelectedNodesChanged);
	InEvents.OnCreateActionMenu = SGraphEditor::FOnCreateActionMenu::CreateSP(this, &FQuestAssetEditorToolkit::OnCreateGraphActionMenu);

	return SNew(SGraphEditor)
		.AdditionalCommands(GraphEditorCommands)
		.IsEditable(true)
		.Appearance(AppearanceInfo)
		.GraphToEdit(CurrentQuestAsset->GetGraph())
		.GraphEvents(InEvents)
		.ShowGraphStateOverlay(false);

}

void FQuestAssetEditorToolkit::CreateObjectiveDetailsWidget()
{
	FDetailsViewArgs DetailsViewArgs;
	DetailsViewArgs.bUpdatesFromSelection = false;
	DetailsViewArgs.bLockable = false;
	DetailsViewArgs.bAllowSearch = true;
	DetailsViewArgs.NotifyHook = this;
	DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::ObjectsUseNameArea;
	DetailsViewArgs.bHideSelectionTip = false;

	FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	ObjectiveDetailsView = PropertyEditorModule.CreateDetailView(DetailsViewArgs);
	ObjectiveDetailsView->SetObjects(PreviousSelectedNodeObjects);
}

void FQuestAssetEditorToolkit::BindEditorCommands()
{
	// Prevent duplicate assigns. This should never happen
	if (GraphEditorCommands.IsValid())
	{
		return;
	}
	GraphEditorCommands = MakeShared<FUICommandList>();

	/* -- Graph Editor Commands -- */
	// Create comment node on graph. Default when you press the "C" key on the keyboard to create a comment.
	GraphEditorCommands->MapAction(
		FGraphEditorCommands::Get().CreateComment,
		FExecuteAction::CreateLambda([this]
			{
				FNewComment_QuestGraphSchemaAction CommentAction;
				CommentAction.PerformAction(CurrentQuestAsset->GetGraph(), nullptr, GraphEditorView->GetPasteLocation());
			})
	);

	GraphEditorCommands->MapAction(
		FGenericCommands::Get().SelectAll,
		FExecuteAction::CreateLambda([this] { GraphEditorView->SelectAllNodes(); }),
		FCanExecuteAction::CreateLambda([] { return true; })
	);

	/* -- Edit Node Commands -- */
	const auto QuestCommands = FQuestCommands::Get();

	//Conversion Commands	
	GraphEditorCommands->MapAction(
		QuestCommands.ConvertObjectiveToKill,
		FExecuteAction::CreateSP(this, &Self::OnCommandConvertObjectiveToKill),
		FCanExecuteAction::CreateLambda([this]
			{
				//TODO: Prüfung nötig?
				return true;
			})
	);

	GraphEditorCommands->MapAction(
		QuestCommands.ConvertObjectiveToCollect,
		FExecuteAction::CreateSP(this, &Self::OnCommandConvertObjectiveToCollect),
		FCanExecuteAction::CreateLambda([this]
			{
				//TODO: Prüfung nötig?
				return true;
			})
	);

	GraphEditorCommands->MapAction(
		QuestCommands.ConvertObjectiveToInteract,
		FExecuteAction::CreateSP(this, &Self::OnCommandConvertObjectiveToInteract),
		FCanExecuteAction::CreateLambda([this]
			{
				//TODO: Prüfung nötig?
				return true;
			})
	);

	GraphEditorCommands->MapAction(
		QuestCommands.ConvertObjectiveToLocate,
		FExecuteAction::CreateSP(this, &Self::OnCommandConvertObjectiveToLocate),
		FCanExecuteAction::CreateLambda([this]
			{
				//TODO: Prüfung nötig?
				return true;
			})
	);

	// Standard Commands
	GraphEditorCommands->MapAction(
		FGenericCommands::Get().Delete,
		FExecuteAction::CreateSP(this, &Self::OnCommandDeleteSelectedNodes),
		FCanExecuteAction::CreateSP(this, &Self::CanDeleteNodes)
	);

	GraphEditorCommands->MapAction(
		FGenericCommands::Get().Copy,
		FExecuteAction::CreateRaw(this, &Self::OnCommandCopySelectedNodes),
		FCanExecuteAction::CreateRaw(this, &Self::CanCopyNodes)
	);

	GraphEditorCommands->MapAction(
		FGenericCommands::Get().Paste,
		FExecuteAction::CreateRaw(this, &Self::OnCommandPasteNodes),
		FCanExecuteAction::CreateRaw(this, &Self::CanPasteNodes)
	);

}


/* -- Node Commands --*/

void FQuestAssetEditorToolkit::OnCommandConvertObjectiveToCollect() const
{
	UE_LOG(LogTemp, Warning, TEXT("Convert Objective(s) To Collect"));
	ConvertNodes<UCollectObjective>();

	CurrentQuestAsset->CompileQuestNodesFromGraphNodes();
	FQuestEditorUtilities::SimpleRefresh(GetQuestGraph(), true);
}

void FQuestAssetEditorToolkit::OnCommandConvertObjectiveToInteract() const
{
	UE_LOG(LogTemp, Warning, TEXT("Convert Objective(s) To Interact"));
	ConvertNodes<UInteractObjective>();

	CurrentQuestAsset->CompileQuestNodesFromGraphNodes();
	FQuestEditorUtilities::SimpleRefresh(GetQuestGraph(), true);
}

void FQuestAssetEditorToolkit::OnCommandConvertObjectiveToKill() const
{
	UE_LOG(LogTemp, Warning, TEXT("Convert Objective(s) To Kill"));
	ConvertNodes<UKillObjective>();
		
	CurrentQuestAsset->CompileQuestNodesFromGraphNodes();
	FQuestEditorUtilities::SimpleRefresh(GetQuestGraph(), true);
}

void FQuestAssetEditorToolkit::OnCommandConvertObjectiveToLocate() const
{
	UE_LOG(LogTemp, Warning, TEXT("Convert Objective(s) To Locate"));
	ConvertNodes<ULocationObjective>();

	CurrentQuestAsset->CompileQuestNodesFromGraphNodes();
	FQuestEditorUtilities::SimpleRefresh(GetQuestGraph(), true);
}

void FQuestAssetEditorToolkit::OnCommandDeleteSelectedNodes() const
{
	const FScopedTransaction Transaction(LOCTEXT("QuestEditRemoveSelectedNode", "Quest Editor: Remove Node"));
	verify(CurrentQuestAsset->Modify());

	const TSet<UObject*> SelectedNodes = GetSelectedNodes();
	UQuestGraph* QuestGraph = GetQuestGraph();
	verify(QuestGraph->Modify());

	// Keep track of all the node types removed
	int32 NumNodesRemoved = 0;
	int32 NumQuestNodesRemoved = 0;
	int32 NumBaseQuestNodesRemoved = 0;

#if DO_CHECK
	const int32 Initial_QuestNodesNum = CurrentQuestAsset->GetObjectives().Num();
	const int32 Initial_GraphNodesNum = QuestGraph->Nodes.Num();
	const int32 Initial_GraphQuestNodesNum = QuestGraph->GetAllQuestGraphNodes().Num();
#endif

	// Unselect nodes we are about to delete
	ClearViewportSelection();

	// Disable compiling for optimization
	CurrentQuestAsset->DisableCompileQuest();

	// Helper function to also count the number of removed nodes
	auto RemoveGraphNode = [&NumNodesRemoved](UEdGraphNode* NodeToRemove)
	{
		verify(FQuestEditorUtilities::RemoveNode(NodeToRemove));
		NumNodesRemoved++;
	};

	for (UObject* NodeObject : SelectedNodes)
	{
		UEdGraphNode* SelectedNode = CastChecked<UEdGraphNode>(NodeObject);
		if (!SelectedNode->CanUserDeleteNode())
		{
			continue;
		}

		// Base Node type
		if (UQuestGraphNode_Base* NodeBase = Cast<UQuestGraphNode_Base>(SelectedNode))
		{
			if (UQuestGraphNode* Node = Cast<UQuestGraphNode>(NodeBase))
			{
				//Don't delete Root!
				if (Node->GetQuestNodeIndex() != 0)
				{
					UE_LOG(LogTemp, Warning, TEXT("Deleting the following Node-Idx: %d"), Node->GetQuestNodeIndex());

					// Remove the Node
					// No need to recompile as the break node links step will do that for us
					RemoveGraphNode(Node);
					NumBaseQuestNodesRemoved++;
					NumQuestNodesRemoved++;
				}
			}

			// Other node types could be checked here			
		}
		else
		{
			// Most likely it is a comment
			RemoveGraphNode(SelectedNode);
		}
	}

	CurrentQuestAsset->EnableCompileQuest();
	if (NumBaseQuestNodesRemoved > 0)
	{
		CurrentQuestAsset->CompileQuestNodesFromGraphNodes();
		CurrentQuestAsset->PostEditChange();
		CurrentQuestAsset->MarkPackageDirty();
		RefreshViewport();
	}

#if DO_CHECK
	// Check if we removed as we counted
	check(Initial_QuestNodesNum - NumQuestNodesRemoved == CurrentQuestAsset->GetObjectives().Num());
	check(Initial_GraphNodesNum - NumNodesRemoved == QuestGraph->Nodes.Num());
	check(Initial_GraphQuestNodesNum - NumQuestNodesRemoved == QuestGraph->GetAllQuestGraphNodes().Num());
#endif
}

bool FQuestAssetEditorToolkit::CanDeleteNodes() const
{
	const TSet<UObject*>& SelectedNodes = GetSelectedNodes();
	// Return false if only root node is selected, as it can't be deleted
	if (SelectedNodes.Num() == 1)
	{
		const UQuestGraphNode* SelectedNode = Cast<UQuestGraphNode>(*FQuestEditorUtilities::GetFirstSetElement(SelectedNodes));

		//Last Element is always root! Element with idx 0 is root! Comments can't be cast to UQuestGraphNode.
		bool IsRoot =
			(SelectedNode != nullptr) &&
			((CurrentQuestAsset->GetObjectives().Num() <= 1) || SelectedNode->GetQuestNodeIndex() == 0);

		//TODO: Maybe add consistency check?
		//do not delete the Root!
		return !IsRoot;
	}

	return SelectedNodes.Num() > 0;

}

void FQuestAssetEditorToolkit::OnCommandCopySelectedNodes() const
{
	// Export the selected nodes and place the text on the clipboard
	const TSet<UObject*>& SelectedNodes = GetSelectedNodes();
	for (UObject* Object : SelectedNodes)
	{
		if (UEdGraphNode* Node = Cast<UEdGraphNode>(Object))
		{
			Node->PrepareForCopying();
		}
	}

	// Copy to clipboard
	FString ExportedText;
	FEdGraphUtilities::ExportNodesToText(SelectedNodes, /*out*/ ExportedText);
	FPlatformApplicationMisc::ClipboardCopy(*ExportedText);

	for (UObject* Object : SelectedNodes)
	{
		if (UQuestGraphNode_Base* Node = Cast<UQuestGraphNode_Base>(Object))
		{
			Node->PostCopyNode();
		}
	}

}

bool FQuestAssetEditorToolkit::CanCopyNodes() const
{
	//TODO: Warum auch immer, kann Root nicht kopiert werden
	// If any of the nodes can be duplicated then we should allow copying
	for (UObject* Object : GetSelectedNodes())
	{
		UEdGraphNode* Node = Cast<UEdGraphNode>(Object);
		if (Node && Node->CanDuplicateNode())
		{
			return true;
		}
	}
	return false;

}

void FQuestAssetEditorToolkit::OnCommandPasteNodes()
{
	PasteNodesHere(GraphEditorView->GetPasteLocation());
}

void FQuestAssetEditorToolkit::PasteNodesHere(const FVector2D& Location)
{
	// Undo/Redo support
	const FScopedTransaction Transaction(FGenericCommands::Get().Paste->GetDescription());
	UQuestGraph* QuestGraph = GetQuestGraph();
	verify(CurrentQuestAsset->Modify());
	verify(QuestGraph->Modify());

	// Clear the selection set (newly pasted stuff will be selected)
	ClearViewportSelection();

	// Disable compiling for optimization
	CurrentQuestAsset->DisableCompileQuest();

	// Grab the text to paste from the clipboard.
	FString TextToImport;
	FPlatformApplicationMisc::ClipboardPaste(TextToImport);

	// Import the nodes
	TSet<UEdGraphNode*> PastedNodes;
	FEdGraphUtilities::ImportNodesFromText(QuestGraph, TextToImport, /*out*/ PastedNodes);

	// Step 1. Calculate average position
	// Average position of nodes so we can move them while still maintaining relative distances to each other
	FVector2D AvgNodePosition(0.0f, 0.0f);
	for (UEdGraphNode* Node : PastedNodes)
	{
		AvgNodePosition.X += Node->NodePosX;
		AvgNodePosition.Y += Node->NodePosY;
	}
	if (PastedNodes.Num() > 0)
	{
		const float InvNumNodes = 1.0f / float(PastedNodes.Num());
		AvgNodePosition.X *= InvNumNodes;
		AvgNodePosition.Y *= InvNumNodes;
	}

	// maps from old index -> new index
	// Used to remap node edges (FDlgEdge) and checking if some TargetIndex edges are still valid
	TMap<int32, int32> OldToNewIndexMap;
	TArray<UQuestGraphNode*> GraphNodes;

	// Step 2. Filter Dialogue nodes and position all valid nodes
	for (UEdGraphNode* Node : PastedNodes)
	{
		bool bAddToSelection = true;
		bool bPositionNode = true;


		if (auto* GraphNode = Cast<UQuestGraphNode>(Node))
		{
			// Add to the dialogue
			const int32 OldNodeIndex = GraphNode->GetQuestNodeIndex();
			const int32 NewNodeIndex = CurrentQuestAsset->AddNode(GraphNode->GetMutableQuestNode());
			GraphNode->SetQuestNodeIndex(NewNodeIndex);

			OldToNewIndexMap.Add(OldNodeIndex, NewNodeIndex);
			GraphNodes.Add(GraphNode);
		}

		// Select the newly pasted stuff
		if (bAddToSelection)
		{
			GraphEditorView->SetNodeSelection(Node, true);
		}

		if (bPositionNode)
		{
			const float NodePosX = (Node->NodePosX - AvgNodePosition.X) + Location.X;
			const float NodePosY = (Node->NodePosY - AvgNodePosition.Y) + Location.Y;

			if (auto* GraphNodeBase = Cast<UQuestGraphNode_Base>(Node))
			{
				GraphNodeBase->SetPosition(NodePosX, NodePosY);
			}
			else
			{
				Node->NodePosX = NodePosX;
				Node->NodePosY = NodePosY;
			}
		}

		// Assign new ID
		Node->CreateNewGuid();
	}

	// Step 3. Fix Edges and references of Dialogue/Graph nodes
	// TODO this is similar to what the compiler does, maybe the compiler is too strict? It should not care what the Dialogues have?
	for (UQuestGraphNode* GraphNode : GraphNodes)
	{
		UObjective* QuestNodeObjective = GraphNode->GetMutableQuestNode();

		const TArray<UQuestGraphNode*> GraphNodeChildren = GraphNode->GetChildNodes();
		const int32 GraphNodeChildrenNum = GraphNodeChildren.Num();

		//TODO: Hier könnten ggf. erhalten Verbindungen zwischen den kopierten Nodes mit kopiert werden.	
		// There are no links copied, delete the edges from the Quest Node.
		QuestNodeObjective->SetNodeChildren({});

	}

	//TODO (Testen): Brauchen wir die ausgeklammerten sachen?
	// First fix weak references
	//FQuestEditorUtilities::RemapOldIndicesWithNewAndUpdateGUID(GraphNodes, OldToNewIndexMap);
	// Compile
	CheckAll();
	CurrentQuestAsset->EnableCompileQuest();
	CurrentQuestAsset->CompileQuestNodesFromGraphNodes();

	// Notify objects of change
	RefreshViewport();
	CurrentQuestAsset->PostEditChange();
	CurrentQuestAsset->MarkPackageDirty();

}

bool FQuestAssetEditorToolkit::CanPasteNodes() const
{
	FString ClipboardContent;
	FPlatformApplicationMisc::ClipboardPaste(ClipboardContent);

	return FEdGraphUtilities::CanImportNodesFromText(CurrentQuestAsset->GetGraph(), ClipboardContent);

}

/* -- Events -- */
void FQuestAssetEditorToolkit::OnSelectedNodesChanged(const TSet<UObject*>& NewSelection)
{
	UE_LOG(LogTemp, Display, TEXT("Selected Nodes changed... ."));
	TArray<UObject*> ViewSelection;

	PreviousSelectedNodeObjects.Empty();

	if (NewSelection.Num())
	{
		for (UObject* Selected : NewSelection)
		{
			PreviousSelectedNodeObjects.Add(Selected);
			ViewSelection.Add(Selected);
		}
	}

	// View the selected objects
	if (ObjectiveDetailsView.IsValid())
	{
		ObjectiveDetailsView->SetObjects(ViewSelection, /*bForceRefresh=*/ true);
	}
}

FActionMenuContent FQuestAssetEditorToolkit::OnCreateGraphActionMenu(UEdGraph* InGraph, const FVector2D& InNodePosition, const TArray<UEdGraphPin*>& InDraggedPins, bool bAutoExpand, SGraphEditor::FActionMenuClosed InOnMenuClosed)
{
	TSharedRef<SQuestActionMenu> ActionMenu = SNew(SQuestActionMenu)
		.Graph(InGraph)
		.NewNodePosition(InNodePosition)
		.DraggedFromPins(InDraggedPins)
		.AutoExpandActionMenu(bAutoExpand)
		.OnClosedCallback(InOnMenuClosed)
		.OnCloseReason(this, &Self::OnGraphActionMenuClosed);

	return FActionMenuContent(ActionMenu, ActionMenu->GetFilterTextBox());


}

void FQuestAssetEditorToolkit::OnGraphActionMenuClosed(bool bActionExecuted, bool bGraphPinContext)
{
	//TODO (Checked): Brauchen wir hier die Sachen aus dem Dialogue?
}

void FQuestAssetEditorToolkit::OnNodeTitleCommitted(const FText& NewText, ETextCommit::Type CommitInfo, UEdGraphNode* NodeBeingChanged) const
{
	if (!IsValid(NodeBeingChanged))
	{
		return;
	}

	//Rename the node to the new set text
	const FScopedTransaction Transaction(LOCTEXT("RenameNode", "Quest Editor: Rename Node"));
	verify(NodeBeingChanged->Modify());
	NodeBeingChanged->OnRenameNode(NewText.ToString());
}



#undef LOCTEXT_NAMESPACE