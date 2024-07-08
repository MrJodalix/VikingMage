// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "QuestAsset/Private/Objective.h"

#include "UObject/Object.h"
#include "IQuestEditorAccess.h"

#include "QuestAsset.generated.h"

USTRUCT(BlueprintType)
struct FQuestData
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool IsComplete;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<FObjectiveData> ObjectiveData;
};

/**
 * A Quest.
 */
UCLASS(BlueprintType, hidecategories = (Object))
class QUESTASSET_API UQuestAsset
	: public UObject
{
	GENERATED_BODY()

public:
	UQuestAsset()
	{
		AssetPath = GetPathName();
	}

	
	/** Gets the QuestData including Objective Data. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
	FQuestData GetQuestData();

	/** Sets the QuestData including the Objectives Data */
	UFUNCTION(BlueprintCallable, Category = "Quest")
	void SetQuestData(FQuestData Data);
	
	/* ---- Quest Functions and Fields ----- */

	// Creates an instance of a quest that won't change the asset during runtime!
	// **ALWAYS USE THIS** otherwise your created quests will change their status during runtime and
	// save that!
	UFUNCTION(BlueprintCallable, Category = "Quest")
		UQuestAsset* CreateInstanceOf(UObject* Outer);

	// Update Quest with the killed enemy
	UFUNCTION(BlueprintCallable, Category = "Quest")
		void EnemyKilled(AActor* enemy);

	// Update Quest with the collected item(s)
	UFUNCTION(BlueprintCallable, Category = "Quest")
		void ItemCollected(int ItemID, int ItemAmount);

	// Update Quest with the visited location
	UFUNCTION(BlueprintCallable, Category = "Quest")
		void LocationVisited(AActor* Location);

	// Update Quest with the interacted actor
	UFUNCTION(BlueprintCallable, Category = "Quest")
		void InteractedWith(AActor* Interactor);

	// Update Quest with the interacted actor
	UFUNCTION(BlueprintCallable, Category = "Quest")
		void TriggeredInteractionID(FName InteractionID);

	// Checks if a certain Objective identified by the ID is Visible, false if not in Visible Objectives
	UFUNCTION(BlueprintCallable, Category = "Quest")
		bool IsObjectiveVisible(FName ObjectiveID);

	// Checks if a certain Objective identified by the ID is Complete, false if no Complete Objective with this ID found
	UFUNCTION(BlueprintCallable, Category = "Quest")
		bool IsObjectiveComplete(FName ObjectiveID);

	// Checks if this quest contains a certain objective
	UFUNCTION(BlueprintCallable, Category = "Quest")
		bool ContainsObjective(FName ObjectiveID);


	UFUNCTION(BlueprintCallable, Category = "Quest")
		void SetObjectiveState(int ObjectiveIdx, EObjectiveState NewState)
	{
		if (IsValidNodeIndex(ObjectiveIdx))
		{
			Objectives[ObjectiveIdx]->State = NewState;
		}
	}

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Quest")
		FString AssetPath;
	
	/** Die ID der Quest */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Quest")
		FName ID;

	/** Quests Title */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Quest")
		FText Title;

	/** Die Beschreibung der Quest */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Quest")
		FText Description;

	/** Marks if the quest is a story quest (essential for story progress) */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Quest")
		bool IsStoryQuest;

	/** Unique identifier for each questasset. Used to uniquely identify a Quest, instead of using the name...*/
	UPROPERTY(VisibleAnywhere, Category = "Quest")
		FGuid GUID;

	/** Ist die Quest abgeschlossen? */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Quest")
		bool IsComplete;

	//TODO (Testen): Was passiert, wenn im Details Panel direkt ein Objective erzeugt wird?
	 /** Die Objectives der Quest */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest")
		TArray<UObjective*> Objectives;

	/* ***** END Quest Functions and Fields ***** */
	////////////////////////////////////////////////////////////////////////////////////////////


public:
	// ----- UObject Interface ----- //

	/**
	 * @return a one line description of an object for viewing in the thumbnail view of the generic browser
	 */
	virtual FString GetDesc() override { return TEXT("DESCRIPTION = ") + GetName(); }

	/**
	 * Presave function. Gets called once before an object gets serialized for saving. This function is necessary
	 * for save time computation as Serialize gets called three times per object from within SavePackage.
	 *
	 * @warning: Objects created from within PreSave will NOT have PreSave called on them!!!
	 */
	virtual void PreSave(const ITargetPlatform* TargetPlatform) override;

	/** UObject serializer. */
	void Serialize(FArchive& Ar) override;

	/**
	 * Do any object-specific cleanup required immediately after loading an object,
	 * and immediately after any undo/redo.
	 */
	void PostLoad() override;

	/**
	 * Called after the C++ constructor and after the properties have been initialized, including those loaded from config.
	 * mainly this is to emulate some behavior of when the constructor was called after the properties were initialized.
	 * This creates the Quest Graph for this Quest.
	 */
	void PostInitProperties() override;

	/** Executed after Rename is executed. */
	void PostRename(UObject* OldOuter, FName OldName) override;

	/**
	 * Called after duplication & serialization and before PostLoad. Used to e.g. make sure UStaticMesh's UModel gets copied as well.
	 * Note: NOT called on components on actor duplication (alt-drag or copy-paste).  Use PostEditImport as well to cover that case.
	 */
	void PostDuplicate(bool bDuplicateForPIE) override;

	/**
	* Called after importing property values for this object (paste, duplicate or .t3d import)
	* Allow the object to perform any cleanup for properties which shouldn't be duplicated or
	* are unsupported by the script serialization
	*/
	void PostEditImport() override;

#if WITH_EDITOR
	/**
	 * Note that the object will be modified.  If we are currently recording into the
	 * transaction buffer (undo/redo), save a copy of this object into the buffer and
	 * marks the package as needing to be saved.
	 *
	 * @param	bAlwaysMarkDirty	if true, marks the package dirty even if we aren't
	 *								currently recording an active undo/redo transaction
	 * @return true if the object was saved to the transaction buffer
	 */
	bool Modify(bool bAlwaysMarkDirty = true) override;

	/**
	 * Called when a property on this object has been modified externally
	 *
	 * @param PropertyChangedEvent the property that was modified
	 */
	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

	/**
	 * This alternate version of PostEditChange is called when properties inside structs are modified.  The property that was actually modified
	 * is located at the tail of the list.  The head of the list of the FStructProperty member variable that contains the property that was modified.
	 */
	void PostEditChangeChainProperty(struct FPropertyChangedChainEvent& PropertyChangedEvent) override;

	/**
	 * Callback used to allow object register its direct object references that are not already covered by
	 * the token stream.
	 *
	 * @param InThis Object to collect references from.
	 * @param Collector	FReferenceCollector objects to be used to collect references.
	 */
	static void AddReferencedObjects(UObject* InThis, FReferenceCollector& Collector);


	// ----- END UObject ----- //


	/* ---- Editor Functions ---- */

	// Broadcasts whenever a property of this quest changes.
	DECLARE_EVENT_OneParam(UQuestAsset, FQuestPropertyChanged, const FPropertyChangedEvent& /* PropertyChangedEvent */);
	FQuestPropertyChanged OnQuestPropertyChanged;

	// Create the basic dialogue graph.
	void CreateGraph();

	// Clears all nodes from the graph.
	void ClearGraph();

	// Gets the editor graph of this quest
	UEdGraph* GetGraph()
	{
		check(QuestGraph);
		return QuestGraph;
	}
	const UEdGraph* GetGraph() const
	{
		//TODO: Why is Graph null after restarting the engine?
		check(QuestGraph);
		return QuestGraph;
	}

	// Gets the quest editor implementation.
	static TSharedPtr<IQuestEditorAccess> GetQuestEditorAccess() { return QuestEditorAccess; }

	// Sets the quest editor implementation. This is called in the constructor of the QuestGraph in the QuestSystemEditor module.
	static void SetQuestEditorAccess(const TSharedPtr<IQuestEditorAccess>& InQuestEditor)
	{
		check(!QuestEditorAccess.IsValid());
		check(InQuestEditor.IsValid());
		QuestEditorAccess = InQuestEditor;
	}

	// Enables/disables the compilation of the dialogues in the editor, use with care. Mainly used for optimization.
	void EnableCompileQuest() { bCompileQuest = true; }
	void DisableCompileQuest() { bCompileQuest = false; }

	// Useful for initially compiling the Quest when we need the extra processing steps done by the compiler.
	void InitialCompileDialogueNodesFromGraphNodes()
	{
		if (bWasCompiledAtLeastOnce)
			return;

		CompileQuestNodesFromGraphNodes();
		bWasCompiledAtLeastOnce = true;
	}

#endif //# if WITH_EDITOR

	//Construct and initialize a node within this Quest
	template<class T>
	T* ConstructQuestNode(TSubclassOf<UObjective> QuestNodeClass = T::StaticClass())
	{
		//Set flag to be transactional so it registers with undo system
		T* QuestNode = NewObject<T>(this, QuestNodeClass, NAME_None, RF_Transactional);
		QuestNode->SetParentQuest(this);
		return QuestNode;
	}

	// Updates the data of some nodes
	// Fills the DlgData with the updated data
	// NOTE: this can do a dialogue data -> graph node data update
	void UpdateAndRefreshData(bool bUpdateTextsNamespacesAndKeys = false);


	//Adds a new node to this quest, returns the index location of the added node in the Nodes array
	int32 AddNode(UObjective* NodeToAdd) { return Objectives.Add(NodeToAdd); }



	// Compiles the quest nodes from the graph nodes. Meaning it transforms the graph data -> (into) quest data.
	void CompileQuestNodesFromGraphNodes();

	// NOTE: don't call this if you don't know what you are doing, you most likely need to call
	// SetStartNode
	// SetNodes
	// After this
	void EmptyNodesGUIDToIndexMap() { NodesGUIDToIndexMap.Empty(); }

	// Sets the Dialogue Nodes. Use with care.
	void SetNodes(const TArray<UObjective*>& InNodes);

	// Sets a new Start Node. Use with care.
	void SetStartNode(UObjective* InStartNode);


private:
	// Updates NodesGUIDToIndexMap with Node
	void UpdateGUIDToIndexMap(const UObjective* Node, int32 NodeIndex);


public:
	/* ---- Blueprint ---- */

	UFUNCTION(BlueprintPure, Category = "Quest")
		FName GetQuestFName() const { return ID; }

	UFUNCTION(BlueprintPure, Category = "Quest")
		FText GetQuestTitle() const { return Title; }

	// Gets the unique identifier for this dialogue.
	UFUNCTION(BlueprintPure, Category = "Quest|GUID")
		FGuid GetGUID() const { check(GUID.IsValid()); return GUID; }

	// Regenerate the GUID of this Dialogue
	void RegenerateGUID() { GUID = FGuid::NewGuid(); }

	UFUNCTION(BlueprintPure, Category = "Quest|GUID")
		bool HasGUID() const { return GUID.IsValid(); }

	UFUNCTION(BlueprintPure, Category = "Quest|Objectives")
		TArray<UObjective*> GetObjectives() const { return Objectives; }

	UFUNCTION(BlueprintPure, Category = "Quest")
		bool IsValidNodeIndex(int32 NodeIndex) const { return Objectives.IsValidIndex(NodeIndex); }

	// Gets the Start Node as a mutable pointer.
	UFUNCTION(BlueprintPure, Category = "Quest", DisplayName = "Get Start Node")
		UObjective* GetMutableStartObjective() const { return StartNode; }
	const UObjective& GetStartObjective() const {
		return *StartNode;// return Objectives.Num() > 0 ? Objectives[0] : nullptr;
	}

	// Gets all active/visible objectives
	UFUNCTION(BlueprintPure, Category = "Quest")
		TArray<UObjective*> GetVisibleObjectives()
	{
		TArray<UObjective*> Result;

		for (UObjective* Objective : Objectives)
		{
			if (Objective->State == EObjectiveState::VISIBLE)
			{
				Result.Add(Objective);
			}
		}
		return Result;
	}

	// Gets all active AND solvable (but hidden) objectives
	UFUNCTION(BlueprintPure, Category = "Quest")
		TArray<UObjective*> GetActiveSolvableObjectives()
	{
		TArray<UObjective*> Result;

		for (UObjective* Objective : Objectives)
		{
			if (Objective->State == EObjectiveState::VISIBLE || Objective->State == EObjectiveState::SOLVABLE)
			{
				Result.Add(Objective);
			}
		}
		return Result;
	}

	// Gets all active AND solvable (but hidden) objectives
	UFUNCTION(BlueprintPure, Category = "Quest")
		TArray<UObjective*> GetObjectivesWithState(EObjectiveState State)
	{
		TArray<UObjective*> Result;

		for (UObjective* Objective : Objectives)
		{
			if (Objective->State == State)
			{
				Result.Add(Objective);
			}
		}
		return Result;
	}



	/* ***** END Blueprint **** */


protected:

	// Maps Node GUID => Node Index
	UPROPERTY(VisibleAnywhere, AdvancedDisplay, Category = "Quest", DisplayName = "Nodes GUID To Index Map")
		TMap<FGuid, int32> NodesGUIDToIndexMap;

	// Root node, Dialogue is started from the first child with satisfied condition (like the SelectorFirst node)
	// NOTE: Add VisibleAnywhere to make it easier to debug
	UPROPERTY(Instanced)
		UObjective* StartNode;

#if WITH_EDITORONLY_DATA
	//EdGraph based representation of the QuestAsset
	UPROPERTY()
		UEdGraph* QuestGraph;


	// Ptr to interface to quest editor operations. See function SetQuestEditorAccess for more details.
	static TSharedPtr<IQuestEditorAccess> QuestEditorAccess;

	// Flag used for optimization, used to enable/disable compiling of the quest for bulk operations.
	bool bCompileQuest = true;

	// Flag indicating if this Quest was compiled at least once in the current runtime.
	bool bWasCompiledAtLeastOnce = false;

#endif


	// Flag that indicates that This Was Loaded was called
	bool bWasLoaded = false;

};
