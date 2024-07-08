// Fill out your copyright notice in the Description page of Project Settings.


#include "VectorBlueprintLibrary.h"
#include "Logging/MessageLog.h"



float UVectorBlueprintLibrary::CalcAngleBetweenVectors(const FVector A, const FVector B)
{	
	return FMath::RadiansToDegrees(acosf(FVector::DotProduct(A, B) * (A.Size() / B.Size())));
}

float UVectorBlueprintLibrary::CalcAngleBetweenVectors2D(const FVector2D A, const FVector2D B)
{
	return FMath::RadiansToDegrees(acosf(FVector2D::DotProduct(A, B) * (A.Size() / B.Size())));
}


FRotator UVectorBlueprintLibrary::AlignRotationToPlane(FRotator InRotation, FVector PlaneNormal) {
	FVector NewUp = UKismetMathLibrary::ProjectVectorOnToPlane(UKismetMathLibrary::GetUpVector(InRotation), PlaneNormal);
	FVector NewRight = UKismetMathLibrary::ProjectVectorOnToPlane(UKismetMathLibrary::GetRightVector(InRotation), PlaneNormal);

	return UKismetMathLibrary::MakeRotationFromAxes(PlaneNormal, NewRight, NewUp);
}



void UVectorBlueprintLibrary::PrintStringErrorLog(FString msg)
{	
	FMessageLog PIELogger = FMessageLog(FName("PIE"));
	PIELogger.Error(FText::FromString(msg));	
}

void UVectorBlueprintLibrary::CustomMoveComponentTo(USceneComponent* Component, FVector TargetRelativeLocation, bool bInterpolateLocation,  FRotator TargetRelativeRotation, bool bInterpolateRotation, bool bEaseOut, bool bEaseIn, float OverTime, bool bForceShortestRotationPath, TEnumAsByte<EMoveComponentAction::Type> MoveAction, FLatentActionInfo LatentInfo)
{
	if (UWorld* World = GEngine->GetWorldFromContextObject(Component, EGetWorldErrorMode::LogAndReturnNull))
	{
		FLatentActionManager& LatentActionManager = World->GetLatentActionManager();
		FInterpolateComponentToAction* Action = LatentActionManager.FindExistingAction<FInterpolateComponentToAction>(LatentInfo.CallbackTarget, LatentInfo.UUID);

		const FVector ComponentLocation = (Component != NULL) ? Component->GetRelativeLocation() : FVector::ZeroVector;
		const FRotator ComponentRotation = (Component != NULL) ? Component->GetRelativeRotation() : FRotator::ZeroRotator;

		// If not currently running
		if (Action == NULL)
		{
			if (MoveAction == EMoveComponentAction::Move)
			{
				// Only act on a 'move' input if not running
				Action = new FInterpolateComponentToAction(OverTime, LatentInfo, Component, bEaseOut, bEaseIn, bForceShortestRotationPath);

				Action->bInterpLocation = bInterpolateLocation;
				Action->bInterpRotation = bInterpolateRotation;

				Action->TargetLocation = TargetRelativeLocation;
				Action->TargetRotation = TargetRelativeRotation;

				Action->InitialLocation = ComponentLocation;
				Action->InitialRotation = ComponentRotation;

				LatentActionManager.AddNewAction(LatentInfo.CallbackTarget, LatentInfo.UUID, Action);
			}
		}
		else
		{
			if (MoveAction == EMoveComponentAction::Move)
			{
				// A 'Move' action while moving restarts interpolation
				Action->TotalTime = OverTime;
				Action->TimeElapsed = 0.f;

				Action->TargetLocation = TargetRelativeLocation;
				Action->TargetRotation = TargetRelativeRotation;

				Action->InitialLocation = ComponentLocation;
				Action->InitialRotation = ComponentRotation;
			}
			else if (MoveAction == EMoveComponentAction::Stop)
			{
				// 'Stop' just stops the interpolation where it is
				Action->bInterpolating = false;
			}
			else if (MoveAction == EMoveComponentAction::Return)
			{
				// Return moves back to the beginning
				Action->TotalTime = Action->TimeElapsed;
				Action->TimeElapsed = 0.f;

				// Set our target to be our initial, and set the new initial to be the current position
				Action->TargetLocation = Action->InitialLocation;
				Action->TargetRotation = Action->InitialRotation;

				Action->InitialLocation = ComponentLocation;
				Action->InitialRotation = ComponentRotation;
			}
		}
	}
}


