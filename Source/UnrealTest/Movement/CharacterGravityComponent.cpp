// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterGravityComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"

enum GravityMovementMode {
	CUSTOM_GRAVITY_WALK, UMETA(DisplayName="Custom Gravity Walking")
	CUSTOM_GRAVITY_FALL, UMETA(DisplayName="Custom Gravity Falling")
	CUSTOM_GRAVITY_JUMP, UMETA(DisplayName="Custom Gravity Jumping")
};

UCharacterGravityComponent::UCharacterGravityComponent() {
	// So we can do our own gravity:
	GravityScale = 0;
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
	
	bAutoRegister = true;
	bWantsInitializeComponent = true;
	bAutoActivate = true;
}

void UCharacterGravityComponent::PostLoad() {
	Super::PostLoad();

	if (CharacterOwner) {
		Camera = CharacterOwner->GetComponentByClass<UCameraComponent>();
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("COULD NOT FIND Camera"));
	}
}

FRotator UCharacterGravityComponent::GetRotatorFromGravity(FVector grav) {
	// Primarily, this is about rotating the global down down vector (and everything else) to match the new gravity vector.
	// So, when the gravity changes, look at how you can set the actor's rotation to match the new gravity.

	FVector normalGrav = grav.GetSafeNormal();

	// We want to use the universal down of gravity to figure out how to set the player's rotation. Otherwise, we'll be setting global rotation
	// based on information calculated from local rotation:
	FVector downGravCross = FVector::CrossProduct(FVector::DownVector, normalGrav);

	// If the cross product is zero, this means that normalGrav either goes directly up (globally), directly down (globally), or is the 0 vector.
	if (downGravCross.IsNearlyZero()) {
		// If the normalGrav is the zero vector, don't make any rotations.
		// Otherwise, just pick the forward vector as the angle we want to rotate along.
		if (!normalGrav.IsNearlyZero()) {
			downGravCross = FVector::ForwardVector;
		}
	}

	// In case downGravCross happens to be the zero vector and gets changed:
	FVector actualCross = FVector::CrossProduct(FVector::DownVector, normalGrav);

	//UE_LOG(LogTemp, Warning, TEXT("Vector we're rotating along: %s Cross product: %s Dot Product: %f"), *downGravCross.ToString(), *actualCross.ToString(), FVector::DotProduct(FVector::DownVector, normalGrav));

	// This is based on the sin(theta) = |cross(A, B)|/|A|*|B| and cos(theta) = dot(A, B)/|A|*|B|
	float crossAngle = FMath::Atan2(actualCross.Size(), FVector::DotProduct(FVector::DownVector, normalGrav)) * 180 / PI; // We need to convert to degrees.

	// Simplified from the nightmare that was before.
	// We create an axis angle representation of our rotation, use the KismetMathLibrary to do the hard job of converting (https://en.wikipedia.org/wiki/Rotation_formalisms_in_three_dimensions#Conversion_formulae_between_formalisms)
	// And then we apply that rotation to our actor.
	FVector axisAngle = downGravCross.GetSafeNormal();

	FRotator newRot = UKismetMathLibrary::RotatorFromAxisAndAngle(axisAngle, crossAngle);

	return newRot;
}

void UCharacterGravityComponent::GravityShift(FVector newGravity) {
	previousGravity = internalGravity;
	internalGravity = newGravity;

	gravityRotation = GetRotatorFromGravity(newGravity);
	previousRotation = GetActorTransform().Rotator();

	gravityRotationCompletion = 0.0f;
}

void UCharacterGravityComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) {
	Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);
}

void UCharacterGravityComponent::CustomGravityWalk() {
}

void UCharacterGravityComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) {
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (MovementMode.GetValue() == EMovementMode::MOVE_Custom) {
		AddForce(internalGravity * 10000.0f);
		switch (CustomMovementMode) {
			case CUSTOM_GRAVITY_WALK:
				CustomGravityWalk();
				break;
		}
	}
	else {
		AddForce(FVector::DownVector * 9.8f * 10000.0f);
	}
}

// For applying forces once they've been set up:
void UCharacterGravityComponent::PhysCustom(float DeltaTime, int32 Iterations) {
	Super::PhysCustom(DeltaTime, Iterations);

	if (gravityRotationCompletion < 1) {
		gravityRotationCompletion = DeltaTime * GravityRotationRate;
		if (gravityRotationCompletion >= 1) {
			gravityRotationCompletion = 1;
		}

		FRotator newRotation = (1 - gravityRotationCompletion) * previousRotation + gravityRotation * gravityRotationCompletion;
		if (Camera) {
			Camera.Get()->SetWorldRotation(newRotation);
		}
	}

	FVector delta = Velocity * DeltaTime;
	FHitResult Adjustment(1.f);
	SafeMoveUpdatedComponent(delta, FRotator::ZeroRotator, true, Adjustment);
}