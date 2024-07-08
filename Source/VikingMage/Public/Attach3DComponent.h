// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "Attach3DComponent.generated.h"

/**
 * A Component Used as a ColliderBox for 3DDragActors
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class VIKINGMAGE_API UAttach3DComponent : public UBoxComponent
{
	GENERATED_BODY()

public:

	/** Detach this Component from its AttachTo
	*
	* @return Actor we were previosly Attachedto
	*/
	UFUNCTION(BlueprintCallable)
	AActor* Detach();

	/** Attach this Component to another 3DAttachComponent
	* 
	*	@return true, if components could be attached
	*/
	UFUNCTION(BlueprintCallable)
	bool Attach(UAttach3DComponent* other);

	/** If this Component is correctly Attached, i.e it has its AttachTarget attached 
	*
	* @return true, if attached correctly
	*/
	UFUNCTION(BlueprintCallable)
	bool IsCorrectlyAttached();

protected:
	/** Starts the Timer to allow Collision after CollisionCooldown Seconds*/
	void AllowCollisionDelayed();

private: 
	/** Allows Collision immedeately (called at End of CollisionCooldown)*/
	UFUNCTION()
	void AllowCollision();

protected: 
	/** The Tag the attached Component has to have to be considered correct */
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FName AttachTargetTag;

	/** The currently attached Component */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	UAttach3DComponent* AttachedTo;

	/** The Time it takes for this Component to collide again after detachment */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	float CollisionCooldown = 1.0f;
};
