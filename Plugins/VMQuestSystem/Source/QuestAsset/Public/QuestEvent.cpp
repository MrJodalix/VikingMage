//TODO: Copyright

#include "QuestEvent.h"

void FQuestEvent::CallEnter() const
{
	if (EventType == EQuestEventType::Custom) {
		if (CustomEvent == nullptr)
		{
			UE_LOG(LogTemp, Warning, TEXT("Custom Event is empty (not valid). Ignoring."));
			return;
		}
		CustomEvent->EnterEvent();
		return;
	}
}

void FQuestEvent::CallExit() const
{
	if (EventType == EQuestEventType::Custom)
	{


		if (CustomEvent == nullptr)
		{
			UE_LOG(LogTemp, Warning, TEXT("Custom Event is empty (not valid). Ignoring."));
			return;
		}
		CustomEvent->ExitEvent();
		return;
	}
}