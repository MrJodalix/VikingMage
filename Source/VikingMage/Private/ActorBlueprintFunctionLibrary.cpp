// Fill out your copyright notice in the Description page of Project Settings.


#include "ActorBlueprintFunctionLibrary.h"

bool UActorBlueprintFunctionLibrary::IsNetStatupActor(AActor* Actor)
{
	return Actor->IsNetStartupActor();
}
