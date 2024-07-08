// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "IKFootPlacerComponent.generated.h"


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class VIKINGMAGE_API UIKFootPlacerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UIKFootPlacerComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:

	/* --- Functions --- */

	/* UACtor */

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


	/* Custom */

	/**
	 * TODO: Comment
	 */
	void UpdateFoot(AActor* OwningActor, float DeltaTime);

	/* ---- Fields ---- */

	/**
	 * Distance a foot travels during one step.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Animation")
		float StepDistance = 20.0f;
	/*
	 * Max Height, the foot reaches during interpolation.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Animation")
		float StepHeight = 20.0f;

	/**
	 * Speed the foot moves to the desired position
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Animation")
		float StepSpeed = 5.0f;

	/**
	 * Distance from the foot to the center line. Could be also called RightVectorOffset
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Animation")
		float FootSpacing = 20.0f;
	/**
	 * Offset for Default Position of the front foots from the center.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Animation")
		float ForwardFootOffset = 50.0f;
	/**
	 * Skeletal Socket of the  foot. TODO: Delete?
	 */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Animation")
		FName FootSocket;

	/**
	 * Current Position of the foot.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Animation")
		FVector CurrentFootPosition;


	/**
	 * Old foot position
	 */
	FVector OldFootPosition;
	/**
	 * New or desired foot position
	 */
	FVector NewFootPosition;

	float Lerp = 0.0f;

};
