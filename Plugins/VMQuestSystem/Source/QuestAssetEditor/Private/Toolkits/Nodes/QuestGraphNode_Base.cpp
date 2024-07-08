//TODO: Copyright
#include "QuestGraphNode_Base.h"

#include "Logging/TokenizedMessage.h"

/* ---- UObject interface ---- */

void UQuestGraphNode_Base::PostLoad()
{
	Super::PostLoad();
	RegisterListeners();
}

void UQuestGraphNode_Base::PostDuplicate(bool bDuplicateForPIE)
{
	Super::PostDuplicate(bDuplicateForPIE);

	if (!bDuplicateForPIE)
	{
		CreateNewGuid();
	}
}

void UQuestGraphNode_Base::PostEditImport()
{
	Super::PostEditImport();
	RegisterListeners();
}

/* ***** END UObject interface ***** */


/* ----- UEdGraphNode interface ----- */

void UQuestGraphNode_Base::AllocateDefaultPins()
{
	// Only one input and output pin
	check(Pins.Num() == 0);
	CreateInputPin();
	CreateOutputPin();
	check(Pins.Num() == 2);
}

void UQuestGraphNode_Base::ReconstructNode()
{
	// Most likely we also need to make sure the new connections are ok
	Modify();

	// Clear previously set messages
	ErrorMsg.Reset();

	// Break any links to 'orphan' pins
	for (UEdGraphPin* Pin : Pins)
	{
		TArray<UEdGraphPin*>& LinkedToRef = Pin->LinkedTo;
		for (UEdGraphPin* OtherPin : LinkedToRef)
		{
			// If we are linked to a pin that its owner doesn't know about, break that link
			if (!OtherPin->GetOwningNode()->Pins.Contains(OtherPin))
			{
				Pin->LinkedTo.Remove(OtherPin);
			}
		}
	}

	// Store the old Input and Output pins
	UEdGraphPin* OldInputPin = HasInputPin() ? GetInputPin() : nullptr;
	UEdGraphPin* OldOutputPin = HasOutputPin() ? GetOutputPin() : nullptr;

	// Move the existing pins to a saved array
	TArray<UEdGraphPin*> OldPins(Pins);
	Pins.Empty();

	// Recreate the new pins
	AllocateDefaultPins();

	// Get new Input and Output pins
	UEdGraphPin* NewInputPin = GetInputPin();
	UEdGraphPin* NewOutputPin = GetOutputPin();

	// Copy data from old to new
	if (OldInputPin)
	{
		NewInputPin->CopyPersistentDataFromOldPin(*OldInputPin);
		OldInputPin = nullptr;
	}

	if (OldOutputPin)
	{
		NewOutputPin->CopyPersistentDataFromOldPin(*OldOutputPin);
		OldOutputPin = nullptr;
	}

	// Throw away the original (old) pins
	for (UEdGraphPin* OldPin : OldPins)
	{
		OldPin->Modify();
		OldPin->BreakAllPinLinks();
		DestroyPin(OldPin);
	}
	OldPins.Empty();
}

/* ***** END UEdGraphNode interface ***** */


/* ----- Own Functions ----- */
UQuestGraphNode_Base::UQuestGraphNode_Base(const FObjectInitializer& ObjectInitializer)
{
	bCanRenameNode = false;
}

bool UQuestGraphNode_Base::HasOutputConnectionToNode(const UEdGraphNode* TargetNode) const
{
	for (UEdGraphPin* ChildInputPin : GetOutputPin()->LinkedTo)
	{
		if (ChildInputPin->GetOwningNode() == TargetNode)
		{
			return true;
		}
	}

	return false;;
}

void UQuestGraphNode_Base::ClearCompilerMessage()
{
	bHasCompilerMessage = false;
	ErrorType = EMessageSeverity::Info;
	ErrorMsg.Empty();
}

void UQuestGraphNode_Base::SetCompilerWarningMessage(FString Message)
{
	bHasCompilerMessage = true;
	ErrorType = EMessageSeverity::Warning;
	ErrorMsg = Message;
}

void UQuestGraphNode_Base::RegisterListeners()
{
	GetQuestAsset()->OnQuestPropertyChanged.AddUObject(this, &UQuestGraphNode_Base::OnQuestPropertyChanged);
}


/* ***** END Own Functions ***** */
