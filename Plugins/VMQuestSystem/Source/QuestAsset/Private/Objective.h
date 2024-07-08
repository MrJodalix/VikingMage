#pragma once

#include "QuestEvent.h"
#include "UObject/Object.h"
#include "Objective.generated.h"

class UQuestAsset;

// Possible states of an obective. Complete or Failed will never be reversed automatically
// by the logic
UENUM(BlueprintType)
enum EObjectiveState
{
	HIDDEN,      // Hidden and Inaktiv
	SOLVABLE,	// Could be completed, but is not visible in QuestLog
	VISIBLE,	 //Visible and Active
	COMPLETE,	 //Completed 
	FAILED		 // Failed Quest
};

UENUM(BlueprintType)
enum EObjectiveEntryCondition
{
	AND,	// All previous Objectives need to be completed
	OR,		// One of the previous Objectives needs to be completed, all previous objectives stay in their current state
	EXOR		// One of the previous Objectives needs to be completed, all previous objectives not completed will fail
};

USTRUCT(BlueprintType)
struct FObjectiveData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		FText Title;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		TEnumAsByte<EObjectiveState>  State;
	
	//CollectObjective
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		int CollectAmount;

	//KillObjective
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		int CurrKillCount;

	FObjectiveData()
	{		
			
	}

};

UCLASS(BlueprintType, Abstract, EditInlineNew, ClassGroup = "Quest")
class QUESTASSET_API UObjective
	: public UObject
{
	GENERATED_BODY()
public:

	UObjective() {}

	/* ----- Graph Functions ----- */

	/** Returns the Title of this objective */
	FText GetObjectiveTitle() { return Title; }
	/** Returns the Type of this objective */
	virtual FText GetObjectiveType() { return FText::FromString("NULL"); }

	/** Returns a description of this objective */
	virtual FText GetObjectiveDescription() { return FText::FromString("Description Missing!"); }

	virtual UObjective* Copy(UObject* Outer, UQuestAsset* NewParent)
	{
		UE_LOG(LogTemp, Error, TEXT("Copy of an Objective should never be called on the parent!"));
		return nullptr;
	}

#if WITH_EDITOR
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

	// Used internally by the Quest editor:
	virtual FString GetNodeTypeString() const { return TEXT("INVALID"); }
#endif //#if WITH_EDITOR

	/* -------------------- */
	/* Child Nodes in Graph */
	/* -------------------- */

	// Gets this nodes children (edges) as a const/mutable array
	UFUNCTION(BlueprintPure, Category = "Quest|Node")
		virtual const TArray<UObjective*>& GetNodeChildren() const { return Children; }
	// Gets this nodes children (edges) as a const/mutable array
	UFUNCTION(BlueprintPure, Category = "Quest|Node")
		virtual const TArray<UObjective*>& GetNodeParents() const { return Parents; }
	//TODO: Muss das virtual sein?
	// Zitat: Eine virtuelle Funktion ist eine Memberfunktion, von der Sie erwarten,
	// dass sie in abgeleiteten Klassen neu definiert wird.

	//Sets an Array of objecitves as the new childs
	virtual void SetNodeChildren(const TArray<UObjective*>& InChildren) { Children = InChildren; }

	// Removes all edges/children
	virtual void RemoveAllChildren() { Children.Empty(); }

	// Adds a new Objective to the end of the Children Array.
	virtual void AddNodeChild(UObjective* InChild) { Children.Add(InChild); }

	//Sets an Array of objecitves as the new parents
	virtual void SetNodeParents(const TArray<UObjective*>& InParents) { Parents = InParents; }

	// Removes all parents
	virtual void RemoveAllParents() { Parents.Empty(); }

	// Adds a new Objective to the end of the Parent Array.
	virtual void AddNodeParent(UObjective* InParent) { Parents.Add(InParent); }

#if WITH_EDITOR
	void SetGraphNode(UEdGraphNode* InNode) { GraphNode = InNode; }
	void ClearGraphNode() { GraphNode = nullptr; }
	UEdGraphNode* GetGraphNode() const { return GraphNode; }
#endif

	/* ---- */
	/* GUID */
	/* ---- */
	UFUNCTION(BlueprintPure, Category = "Quest|Node")
		FGuid GetGUID() const { return NodeGUID; }

	UFUNCTION(BlueprintPure, Category = "Quest|Node")
		bool HasGUID() const { return NodeGUID.IsValid(); }

	void RegenerateGUID()
	{
		NodeGUID = FGuid::NewGuid();
		Modify();
	}

	// Checks if this objective is a valid objective. Only derived classes of UObjective are valid Objectives. 
	virtual bool IsValid() const { return false; }

	/** Broadcasts whenever a property of this objective changes. */
	DECLARE_EVENT_OneParam(UObjective, FObjectivePropertyChanged, const FPropertyChangedEvent& /* PropertyChangedEvent */);
	FObjectivePropertyChanged OnObjectivePropertyChanged;

	// Does Everything that needs to happen if a certain node is entered. Includes firing events.
	virtual bool HandleNodeEnter();

	// Does Everything that needs to happen if a certain node is left. Includes firing events.
	virtual bool HandleNodeExit();

	// Checks if a objective should be marked as active. Considers the Enter Conditions.
	virtual bool CheckObjectiveActiveConditions();

	// Fails all parents not completed
	void EXORParents();

	// Updates the visibility of the children
	void UpdateChildren();

	// Sets the Objective to Visible/Active if it is possible and the
	// Objective is Hidden
	void UpdateVisibility();


	//
	// For the EnterEvents and ExitEvents
	//
	void FireEvents(TArray<FQuestEvent> Events, bool IsEnter);

	//Enter
	UFUNCTION(BlueprintPure, Category = "Quest|Node")
		virtual bool HasAnyEnterEvents() const { return GetNodeEnterEvents().Num() > 0; }

	UFUNCTION(BlueprintPure, Category = "Quest|Node")
		virtual const TArray<FQuestEvent>& GetNodeEnterEvents() const { return EnterEvents; }

	virtual void SetNodeEnterEvents(const TArray<FQuestEvent>& InEnterEvents) { EnterEvents = InEnterEvents; }

	//Exit
	UFUNCTION(BlueprintPure, Category = "Quest|Node")
		virtual bool HasAnyExitEvents() const { return GetNodeExitEvents().Num() > 0; }

	UFUNCTION(BlueprintPure, Category = "Quest|Node")
		virtual const TArray<FQuestEvent>& GetNodeExitEvents() const { return ExitEvents; }

	virtual void SetNodeExitEvents(const TArray<FQuestEvent>& InEnterEvents) { ExitEvents = InEnterEvents; }


	//
	// Updte Objective Status
	//

	UFUNCTION(BlueprintCallable, Category = "Quest|Objective")
		// Sets an Objective to Completed and Updates all the Children.
		void SetObjectiveCompleted()
	{
		State = EObjectiveState::COMPLETE;
		HandleNodeExit();
		UpdateChildren();
	}

	UFUNCTION(BlueprintCallable, Category = "Quest|Objective")
		void SetObjectiveVisible()
	{
		State = EObjectiveState::VISIBLE;
		HandleNodeEnter();
	}

	void SetParentQuest(UQuestAsset* Quest)
	{
		ParentQuest = Quest;
	}

	/** Returns the objective data that could change during the game as a struct. */
	UFUNCTION(BlueprintCallable, Category = "Quest|Objective")
	virtual FObjectiveData GetObjectiveData();

	/** Sets the objective data that could change during the game. */
	UFUNCTION(BlueprintCallable, Category = "Quest|Objective")
	virtual void SetObjectiveData(FObjectiveData Data);
	
	

	/* ----- Properties ----- */

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Objective")
		FName ID;

	/** Die Titel der Quest */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Objective")
		FText Title;

	//TODO: Description anhand der Werte automatisch zurückerhalten?
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Objective")
		FText Description;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
		/** ist das Abschließen dieses Objectives Optional? */
		bool IsQuestEnding;


	UPROPERTY(BlueprintReadOnly, EditAnywhere)
		/** Status des Objectives */
		TEnumAsByte<EObjectiveState> State;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
		/** Entry Condition, considering completed parents, describes when this objective might be active */
		TEnumAsByte<EObjectiveEntryCondition> EntryCondition;

	//TODO: Muss noch überarbeitet werden, irgendeine schöne Auswahlmöglichkeit per GUI muss her
	//TODO: Funktioniert das? Müsste das mit kopiert werden in Copy?
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
		/** Indizes der Objectives, die beim Abschließen dieses Objectives deaktiviert werden sollen*/
		TArray<TSubclassOf<UObjective>> DisableOnCompletion;




protected:
#if WITH_EDITORONLY_DATA
	// Node's Graph representation, used to get position.
	UPROPERTY(Meta = (DlgNoExport))
		UEdGraphNode* GraphNode = nullptr;

	// Used to build the change event and broadcast it
	int32 BroadcastPropertyEdgeIndexChanged = INDEX_NONE;
#endif // WITH_EDITORONLY_DATA

protected:

	// Events fired when the node is reached in the dialogue
	UPROPERTY(EditAnywhere, Category = "Objective|Node")
		TArray<FQuestEvent> EnterEvents;

	// Events fired when the node is reached in the dialogue
	UPROPERTY(EditAnywhere, Category = "Objective|Node")
		TArray<FQuestEvent> ExitEvents;

	// The Unique identifier for each Node. This is much safer than a Node Index.
	// Compile/Save Asset to generate this
	UPROPERTY(VisibleAnywhere, Category = "Objective|Node")
		FGuid NodeGUID;

	//Objectives, following after this objective
	UPROPERTY(VisibleAnywhere, EditFixedSize, Category = "Objective|Node")
		TArray<UObjective*> Children;

	//Objectives, one layer above this objective. Important for conditions, when to activate an objecitve
	UPROPERTY(VisibleAnywhere, EditFixedSize, Category = "Objective|Node")
		TArray<UObjective*> Parents;

	UPROPERTY(VisibleAnywhere, Category = "Objective|Node")
		UQuestAsset* ParentQuest;

};



