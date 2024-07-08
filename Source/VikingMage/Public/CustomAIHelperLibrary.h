// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "CustomAIHelperLibrary.generated.h"

class AAIController;
class UAIAsyncTaskBlueprintProxy;
class UAnimInstance;
class UBehaviorTree;
class UBlackboardComponent;
class UNavigationPath;
class UPathFollowingComponent;

/**
 *
 */
UCLASS()
class VIKINGMAGE_API UCustomAIHelperLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable, Category = "AI|CustomNavigation")
	static void CustomSimpleMoveToLocation(AController* Controller, const FVector& Goal, bool ReachExactGoal, float AcceptanceRadius);

};
