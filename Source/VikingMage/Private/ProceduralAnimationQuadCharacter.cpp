// Fill out your copyright notice in the Description page of Project Settings.


#include "ProceduralAnimationQuadCharacter.h"

#include "Components/CapsuleComponent.h"

// Sets default values
AProceduralAnimationQuadCharacter::AProceduralAnimationQuadCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	LeftFrontFoot = CreateDefaultSubobject<UIKFootPlacerComponent>(TEXT("LeftFrontFoot"));
	RightFrontFoot = CreateDefaultSubobject<UIKFootPlacerComponent>(TEXT("RightFrontFoot"));
	LeftBackFoot = CreateDefaultSubobject<UIKFootPlacerComponent>(TEXT("LeftBackFoot"));
	RightBackFoot = CreateDefaultSubobject<UIKFootPlacerComponent>(TEXT("RightBackFoot"));
			
}

// Called when the game starts or when spawned
void AProceduralAnimationQuadCharacter::BeginPlay()
{
	Super::BeginPlay();

	GetMesh()->SetRelativeLocation(FVector(0, 0, -GetCapsuleComponent()->GetScaledCapsuleHalfHeight()));
}

// Called every frame
void AProceduralAnimationQuadCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	LeftFrontFoot->UpdateFoot(this, DeltaTime);
	RightFrontFoot->UpdateFoot(this, DeltaTime);
	LeftBackFoot->UpdateFoot(this, DeltaTime);
	RightBackFoot->UpdateFoot(this, DeltaTime);
	
}

// Called to bind functionality to input
void AProceduralAnimationQuadCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}



