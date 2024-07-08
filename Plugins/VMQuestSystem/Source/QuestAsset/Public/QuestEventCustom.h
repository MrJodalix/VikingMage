//TODO: Copyright
#pragma once

#include "UObject/Object.h"

#include "QuestEventCustom.generated.h"

// Abstract base class for a custom event
// Extend this class to define additional data you want to store
//
// 1. Override EnterEvent
UCLASS(Blueprintable, BlueprintType, Abstract, EditInlineNew)
class QUESTASSET_API UQuestEventCustom : public UObject
{
	GENERATED_BODY()

public:

	//UObject interface
	void PostInitProperties() override
	{
		// We must always set the outer to be something that exists at runtime
#if WITH_EDITOR
		if (UEdGraphNode* GraphNode = Cast<UEdGraphNode>(GetOuter()))
		{
					
		}
#endif

		Super::PostInitProperties();
	}
	virtual UWorld* GetWorld() const override
	{
		if (HasAnyFlags(RF_ArchetypeObject | RF_ClassDefaultObject))
		{
			return nullptr;
		}

		// Get from outer
		if (UObject* Outer = GetOuter())
		{
			if (UWorld* World = Outer->GetWorld())
			{
				return World;
			}
		}

		// Fallback to default autodetection
		if (GEngine)
		{
			// Get first PIE world
			// Copied from TakeUtils::GetFirstPIEWorld()
			for (const FWorldContext& Context : GEngine->GetWorldContexts())
			{
				UWorld* World = Context.World();
				if (!World || !World->IsPlayInEditor())
					continue;

				if (World->GetNetMode() == ENetMode::NM_Standalone ||
					(World->GetNetMode() == ENetMode::NM_Client && Context.PIEInstance == 2))
				{
					return World;
				}
			}

			// Otherwise get the first Game World
			for (const FWorldContext& Context : GEngine->GetWorldContexts())
			{
				UWorld* World = Context.World();
				if (!World || !World->IsGameWorld())
					continue;

				return World;
			}
		}
		
		//Failed
		UE_LOG(LogTemp, Error, TEXT("Couldn't find a World!"));
		return nullptr;
	}


	// Called when the event is triggered.
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Quest", DisplayName = "Enter")
		void EnterEvent();
	virtual void EnterEvent_Implementation() {}

	// Called when the event is exit.
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Quest", DisplayName = "Exit")
		void ExitEvent();
	virtual void ExitEvent_Implementation() {}
};

UCLASS(Blueprintable, BlueprintType, Abstract, EditInlineNew, CollapseCategories)
class QUESTASSET_API UQuestEventCustomHideCategories : public UQuestEventCustom
{
	GENERATED_BODY()
};