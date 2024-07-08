// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "IKFootPlacerComponent.h"

#include "ProceduralAnimationQuadCharacter.generated.h"

UCLASS()
class VIKINGMAGE_API AProceduralAnimationQuadCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AProceduralAnimationQuadCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:

	/* ---- Functions ---- */

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/* ---- Fields ---- */

	/**
	 * TODO Comment
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Component")
		UIKFootPlacerComponent* LeftFrontFoot;
	/**
	 * TODO Comment
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Component")
		UIKFootPlacerComponent* RightFrontFoot;
	/**
	 * TODO Comment
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Component")
		UIKFootPlacerComponent* LeftBackFoot;
	/**
	 * TODO Comment
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Component")
		UIKFootPlacerComponent* RightBackFoot;




private:

};
