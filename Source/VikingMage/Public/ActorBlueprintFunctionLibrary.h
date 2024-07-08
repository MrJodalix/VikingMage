// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ActorBlueprintFunctionLibrary.generated.h"

/**
 *
 */
UCLASS()
class VIKINGMAGE_API UActorBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

		UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Actor")
		static bool IsNetStatupActor(AActor* Actor);

};
