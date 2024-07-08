//TODO: Copyright
#include "QuestCommands.h"

#define LOCTEXT_NAMESPACE "QuestCommands"

void FQuestCommands::RegisterCommands()
{
	UI_COMMAND(
		ConvertObjectiveToKill,
		"Convert to kill objective",
		"Converts this current selected objective(s) to kill objectiv(s)",
		EUserInterfaceActionType::Button, FInputChord()
	);

	UI_COMMAND(
		ConvertObjectiveToLocate,
		"Convert to locate objective",
		"Converts this current selected objective(s) to locate objectiv(s)",
		EUserInterfaceActionType::Button, FInputChord()
	);

	UI_COMMAND(
		ConvertObjectiveToCollect,
		"Convert to collect objective",
		"Converts this current selected objective(s) to collect objectiv(s)",
		EUserInterfaceActionType::Button, FInputChord()
	);

	UI_COMMAND(
		ConvertObjectiveToInteract,
		"Convert to interact objective",
		"Converts this current selected objective(s) to interact objectiv(s)",
		EUserInterfaceActionType::Button, FInputChord()
	);
}

#undef LOCTEXT_NAMESPACE
