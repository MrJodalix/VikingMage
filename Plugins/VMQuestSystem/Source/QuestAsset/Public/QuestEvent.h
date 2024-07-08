//TODO: Copyright
#pragma once

#include "CoreMinimal.h"
#include "QuestEventCustom.h"

#include "QuestEvent.generated.h"

UENUM(BlueprintType)
enum class EQuestEventType : uint8
{
	// User Defined Event, calls EnterEvent on the custom event object.
	//
	// 1. Create a new Blueprint derived from DlgEventCustom (or DlgEventCustomHideCategories)
	// 2. Override EnterEvent
	Custom						UMETA(DisplayName = "Custom Event")
};

USTRUCT(BlueprintType)
struct QUESTASSET_API FQuestEvent
{
	GENERATED_USTRUCT_BODY()

public:

	bool operator==(const FQuestEvent& Other) const
	{
		//TODO: Implement check if equal!
		return false;
	}

	//Executes the event
	void CallEnter() const;
	void CallExit() const;
		
	FString GetCustomEventName() const
	{
		return CustomEvent ? CustomEvent->GetName() : TEXT("INVALID");
	}


public:	

	// Type of the event, can be a simple event or a call to modify a bool/int/float variable
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue|Event")
		EQuestEventType EventType = EQuestEventType::Custom;
	
	// User Defined Event, calls EnterEvent on the custom event object.
	//
	// 1. Create a new Blueprint derived from QuestEventCustom (or QuestEventCustomHideCategories)
	// 2. Override EnterEvent
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced, Category = "Quest|Event")
		UQuestEventCustom* CustomEvent = nullptr;
	
};
