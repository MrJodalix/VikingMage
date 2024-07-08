//TODO: Copyright
#pragma once

#include "Framework/Commands/Commands.h"

class FQuestCommands : public TCommands<FQuestCommands>
{
public:
	FQuestCommands()
		: TCommands<FQuestCommands>(
			TEXT("QuestSystemEditor"), //Context name for fast lookup
			NSLOCTEXT("Contexts", "QuestSystemEditor", "QuestSystem Editor"), //Localiced context name for displaying
			NAME_None, //Parnet
			FEditorStyle::Get().GetStyleSetName() //Standard Style
			)
	{
	}

	//
	// TCommand<> interface
	//
	void RegisterCommands() override;

public:

	//TODO: Documentation improvements
	// Converts an objective to a kill objective
	TSharedPtr<FUICommandInfo> ConvertObjectiveToKill;
	TSharedPtr<FUICommandInfo> ConvertObjectiveToLocate;
	TSharedPtr<FUICommandInfo> ConvertObjectiveToCollect;
	TSharedPtr<FUICommandInfo> ConvertObjectiveToInteract;

};