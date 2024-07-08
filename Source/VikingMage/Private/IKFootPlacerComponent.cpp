// Fill out your copyright notice in the Description page of Project Settings.


#include "IKFootPlacerComponent.h"

// Sets default values for this component's properties
UIKFootPlacerComponent::UIKFootPlacerComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UIKFootPlacerComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UIKFootPlacerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
		
	// ...
}

void UIKFootPlacerComponent::UpdateFoot(AActor* OwningActor,  float DeltaTime)
{
	FCollisionQueryParams TraceParams = FCollisionQueryParams(FName(TEXT("Trace")), true, 
		OwningActor);

	TraceParams.bTraceComplex = true;
	TraceParams.bReturnPhysicalMaterial = false;

	FHitResult HitResult(ForceInit);
	FVector Start = OwningActor->GetActorLocation() + (OwningActor->GetActorRightVector() * FootSpacing) +
		(OwningActor->GetActorForwardVector() * ForwardFootOffset);
	FVector End = Start + (OwningActor->GetActorUpVector() * -1.0f) * 100.0f;

	if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, TraceParams))
	{
		//Set new position, when next step is at least step distance
		if (FVector::Dist(NewFootPosition, HitResult.ImpactPoint) >= StepDistance)
		{
			Lerp = 0.0f;
			NewFootPosition = HitResult.ImpactPoint;
		}
	}
	if (Lerp < 1.0f)
	{
		FVector FootPosition = FMath::Lerp<FVector>(OldFootPosition, NewFootPosition, Lerp);
		FootPosition.Z += FMath::Sin(Lerp * PI) * StepHeight;

		CurrentFootPosition = FootPosition;
		Lerp += DeltaTime * StepSpeed;
	}
	else
	{
		OldFootPosition = NewFootPosition;
	}
	
	
}

