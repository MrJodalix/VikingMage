// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Kismet/KismetMathLibrary.h"

#include <Runtime/Engine/Private/InterpolateComponentToAction.h>
#include <Runtime/Engine/Classes/Kismet/KismetSystemLibrary.h>
#include "VectorBlueprintLibrary.generated.h"

/**
 * Functions for basic vector operations not included in UE
 */
UCLASS()
class VIKINGMAGE_API UVectorBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Calculates the angle between two vectors in degree */
	UFUNCTION(BlueprintCallable, Category = "Math|Vector")
		static float CalcAngleBetweenVectors(FVector A, FVector B);

	/**Calculates the angle between two vectors in degrees */
	UFUNCTION(BlueprintCallable, Category = "Math|Vector")
		static float CalcAngleBetweenVectors2D(FVector2D A, FVector2D B);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Math|Rotator")
		static FRotator AlignRotationToPlane(FRotator InRotation, FVector PlaneNormal);


	//TODO: UDebugBlueprintLibrary
	UFUNCTION(BlueprintCallable, Category = "Debug|ErrorLog")
		static void PrintStringErrorLog(FString msg);

	/*
	 * Interpolate a component to the specified relative location and rotation over the course of OverTime seconds.
	 * @param Component						Component to interpolate
	 * @param TargetRelativeLocation		Relative target location
	 * @param bInterpolateLocation			if true we will interpolate the Location
	 * @param TargetRelativeRotation		Relative target rotation
	 * @param bInterpolateRotation			if true we will interpolate the Rotation
	 * @param bEaseOut						if true we will ease out (ie end slowly) during interpolation
	 * @param bEaseIn						if true we will ease in (ie start slowly) during interpolation
	 * @param OverTime						duration of interpolation
	 * @param bForceShortestRotationPath	if true we will always use the shortest path for rotation
	 * @param MoveAction					required movement behavior @see EMoveComponentAction
	 * @param LatentInfo					The latent action
	 */
	UFUNCTION(BlueprintCallable, meta = (Latent, LatentInfo = "LatentInfo", WorldContext = "WorldContextObject", bInterpolateLocation = "true", bInterpolateRotation = "true",  ExpandEnumAsExecs = "MoveAction", OverTime = "0.2"), Category = "Components")
		static void CustomMoveComponentTo(USceneComponent* Component, FVector TargetRelativeLocation, bool bInterpolateLocation, FRotator TargetRelativeRotation, bool bInterpolateRotation, bool bEaseOut, bool bEaseIn, float OverTime, bool bForceShortestRotationPath, TEnumAsByte<EMoveComponentAction::Type> MoveAction, FLatentActionInfo LatentInfo);

};
