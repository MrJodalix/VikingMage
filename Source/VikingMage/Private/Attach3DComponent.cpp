// Fill out your copyright notice in the Description page of Project Settings.


#include "Attach3DComponent.h"
#include "Kismet/KismetSystemLibrary.h"

AActor* UAttach3DComponent::Detach() {
	if (AttachedTo) {
		AttachedTo->AttachedTo = NULL;
		AttachedTo->AllowCollisionDelayed();
	}

	AActor* AttachedActor = AttachedTo->GetOwner();

	AttachedTo = NULL;
	this->AllowCollisionDelayed();

	return AttachedActor;
}

bool UAttach3DComponent::Attach(UAttach3DComponent* other) {

	UE_LOG(LogTemp, Warning, TEXT("Attaching %s to %s"), *(UKismetSystemLibrary::GetDisplayName(this)), *(UKismetSystemLibrary::GetDisplayName(other)));

	if (!other) {
		UE_LOG(LogTemp, Warning, TEXT("Attaching to null, should call Detach instead"));
		Detach();
	}

	if (other->AttachedTo != NULL || this->AttachedTo != NULL) {
		UE_LOG(LogTemp, Warning, TEXT("One of the Components is already attached"));
		return false;
	}

	bool IsCompatible = false;
	for (FName tag : ComponentTags) {
		IsCompatible = IsCompatible || other->ComponentHasTag(tag);
	}

	if (!IsCompatible) {
		return false;
	}

	other->AttachedTo = this;
	this->AttachedTo = other;

	this->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	other->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	return true;
}

bool UAttach3DComponent::IsCorrectlyAttached() {
	if (AttachedTo) {
		return AttachedTo->ComponentHasTag(AttachTargetTag);
	}
	return AttachTargetTag == FName("None");
}

void UAttach3DComponent::AllowCollisionDelayed() {
	FTimerHandle TimerHandle = FTimerHandle();
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UAttach3DComponent::AllowCollision, CollisionCooldown);
}

void UAttach3DComponent::AllowCollision() {
	this->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}
