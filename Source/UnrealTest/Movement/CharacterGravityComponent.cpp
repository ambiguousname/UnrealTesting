// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterGravityComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/Character.h"
#include "GameFramework/PhysicsVolume.h"
#include "Components/CapsuleComponent.h"
#include "LandscapeComponent.h"

UCharacterGravityComponent::UCharacterGravityComponent() {
	// So we can do our own gravity:
	GravityScale = 0;
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
	
	bAutoRegister = true;
	bWantsInitializeComponent = true;
	bAutoActivate = true;

	preSkiingCustomMovementMode = CUSTOM_GRAVITY_WALK;
	preSkiingMovementMode = EMovementMode::MOVE_Walking;

	skiTraceParams = FCollisionQueryParams();
	skiTraceParams.AddIgnoredActor(GetOwner());

	skiTraceShape = FCollisionShape::MakeSphere(1.0f);
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

bool UCharacterGravityComponent::RotateTowardsGravity(float DeltaTime, FRotator& out) {
	if (gravityRotationCompletion < 1) {
		gravityRotationCompletion += DeltaTime * GravityRotationRate;
		if (gravityRotationCompletion >= 1) {
			gravityRotationCompletion = 1;
		}

		out = (1 - gravityRotationCompletion) * previousRotation + gravityRotation * gravityRotationCompletion;
		out.Normalize();
		return true;
	}
	else {
		return false;
	}
}

void UCharacterGravityComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) {
	Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);
}

FVector UCharacterGravityComponent::SkiCalcGroundVelocity(float DeltaTime) {
	// TODO, fix scaling so it's more accurate and tied to maximum velocity
	// Acceleration is merely a suggestion (scales up as velocity scales up):
	//FVector accel = Acceleration * skiingGroundInputFactor * Velocity.Length()/1000 * DeltaTime;
	//Velocity += accel;
	return Velocity;
}

FVector UCharacterGravityComponent::GetAverageNormalBeneath() {
	UWorld* const World = GetWorld();
	FVector normal = FVector::ZeroVector;
	if (World) {
		// Either SweepMultiByChannel or OverlapMultiByChannel (I think Sweep since it allows for getting normal hits)
		TArray<FHitResult> outHits;
		World->SweepMultiByChannel(outHits, GetActorFeetLocation(), internalGravity, FQuat::Identity, ECC_WorldDynamic, skiTraceShape);
		for (int32 i = 0; i < outHits.Num(); i++) {
			UE_LOG(LogTemp, Warning, TEXT("%s"), *outHits[i].ImpactNormal.ToString())
			normal.AddBounded(outHits[i].ImpactNormal);
		}
		/*for (int i = 0; i < iterations; i++) {
			float angle = 2 * PI * ((float)i / (float)iterations);
			World->LineTraceSingleByChannel(out, ,  ECC_WorldDynamic);
		}*/
	}
	return normal;
}

void UCharacterGravityComponent::SkiGroundHit(FHitResult Hit) {
	// Here's the real meat and potatoes. Skiing works by taking in the gravity and turning that into pure forward velocity.
	// This is where surfaces come with the assist.
	// Imagine three scenarios:

	// 1. You're standing on a flat surface and skiing.
	// 2. You're on a downslope and your current velocity is headed down the slope. Gravity pulls you down the hill.
	// 3. You're on a downslope and your current velocity is headed up the slope. Gravity pulls you down the hill.
	
	// You don't have any control over where you *want* to slide, so this is all based on gravity. 

	// For 2 and 3, we want to know the force that's pulling us along the slope at all times. SO!
	// We use some physics: https://www.khanacademy.org/science/physics/forces-newtons-laws/inclined-planes-friction/a/what-are-inclines
	// Here's what we want:
	// Given a plane's normal N, and the down direction G, find the vector V along the plane whose dot product with G is closest to 1 as possible.
	// Easy! Project the gravity vector onto the plane.
	// (Because I don't remember everyting from vector calc) https://stackoverflow.com/questions/9605556/how-to-project-a-point-onto-a-plane-in-3d
	FVector gravNormal = internalGravity.GetSafeNormal();

	// I think what we need to do is get the average of nearby normals, so we don't get random jittering:
	FVector normal = GetAverageNormalBeneath();

	double dot = normal.Dot(gravNormal);
	FVector projected = gravNormal - dot * normal;

	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("%s %s %lf"), *normal.ToString(), *projected.ToString(), dot));

	// The closer we are to a flat plane (compared to the gravity), the less impact we'll have on skiing.
	if (dot > -0.98f) {
		Velocity += projected.GetSafeNormal() * skiSlopeAcceleration;
	}
	// And so the next pass that we do movement, this will be added.
	// TODO: Probably set this up earlier.
}

void UCharacterGravityComponent::CustomGravityWalk(float DeltaTime, FRotator newRotation) {
	FVector vel = FVector::ZeroVector;
	if (bIsSkiing) {
		vel = SkiCalcGroundVelocity(DeltaTime);
	}
	else {
		CalcVelocity(DeltaTime, GroundFriction, false, BrakingDecelerationWalking);
		vel = Velocity;
	}
	FVector delta = vel*DeltaTime;
	FVector RampVector = FVector(delta);
	FHitResult Hit(1.f);
	SafeMoveUpdatedComponent(delta, newRotation, true, Hit);
	if (!Hit.IsValidBlockingHit()) {
		SetMovementMode(EMovementMode::MOVE_Custom, CUSTOM_GRAVITY_FALL);
		return;
	}

	if (bIsSkiing) {
		SkiGroundHit(Hit);
	}

	float LastMoveTimeSlice = DeltaTime;
	//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, FString::Printf(TEXT("Blocking: %d Walkable Ramp: %d"), Hit.IsValidBlockingHit(), (Hit.Time > 0.f) && (Hit.Normal.Z > UE_KINDA_SMALL_NUMBER) && IsWalkable(Hit)));

	// Taken wholesale (and modified some beyond that) from CharacterMovementComponent.cpp's MoveAlongFloor:

	// I've yet to see when this is called:
	if (Hit.bStartPenetrating) {
		HandleImpact(Hit);
		SlideAlongSurface(delta, 1.f, Hit.Normal, Hit, true);

		if (Hit.bStartPenetrating)
		{
			OnCharacterStuckInGeometry(&Hit);
		}
	}
	else if (Hit.IsValidBlockingHit()) {
		float PercentTimeApplied = Hit.Time;

		if ((Hit.Time > 0.f) && (Hit.Normal.Z > UE_KINDA_SMALL_NUMBER) && IsWalkable(Hit))
		{
			// Another walkable ramp.
			const float InitialPercentRemaining = 1.f - PercentTimeApplied;
			RampVector = ComputeGroundMovementDelta(delta * InitialPercentRemaining, Hit, false);
			LastMoveTimeSlice = InitialPercentRemaining * LastMoveTimeSlice;
			SafeMoveUpdatedComponent(RampVector, UpdatedComponent->GetComponentQuat(), true, Hit);

			const float SecondHitPercent = Hit.Time * InitialPercentRemaining;
			PercentTimeApplied = FMath::Clamp(PercentTimeApplied + SecondHitPercent, 0.f, 1.f);
		}

		if (Hit.IsValidBlockingHit())
		{
			if (CanStepUp(Hit) || (CharacterOwner->GetMovementBase() != nullptr && Hit.HitObjectHandle == CharacterOwner->GetMovementBase()->GetOwner()))
			{
				// hit a barrier, try to step up
				const FVector PreStepUpLocation = UpdatedComponent->GetComponentLocation();
				const FVector GravDir(0.f, 0.f, -1.f);
				FStepDownResult out;
				if (!StepUp(GravDir, delta * (1.f - PercentTimeApplied), Hit, &out))
				{
					UE_LOG(LogTemp, Verbose, TEXT("- StepUp (ImpactNormal %s, Normal %s"), *Hit.ImpactNormal.ToString(), *Hit.Normal.ToString());
					HandleImpact(Hit, LastMoveTimeSlice, RampVector);
					SlideAlongSurface(delta, 1.f - PercentTimeApplied, Hit.Normal, Hit, true);
				}
				else
				{
					UE_LOG(LogTemp, Verbose, TEXT("+ StepUp (ImpactNormal %s, Normal %s"), *Hit.ImpactNormal.ToString(), *Hit.Normal.ToString());
					if (!bMaintainHorizontalGroundVelocity)
					{
						// Don't recalculate velocity based on this height adjustment, if considering vertical adjustments. Only consider horizontal movement.
						bJustTeleported = true;
						const float StepUpTimeSlice = (1.f - PercentTimeApplied) * DeltaTime;
						if (!HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity() && StepUpTimeSlice >= UE_KINDA_SMALL_NUMBER)
						{
							Velocity = (UpdatedComponent->GetComponentLocation() - PreStepUpLocation) / StepUpTimeSlice;
							Velocity.Z = 0;
						}
					}
				}
			}
			else if (Hit.Component.IsValid() && !Hit.Component.Get()->CanCharacterStepUp(CharacterOwner))
			{
				HandleImpact(Hit, LastMoveTimeSlice, RampVector);
				SlideAlongSurface(delta, 1.f - PercentTimeApplied, Hit.Normal, Hit, true);
			}
		}
	}
}

// READ MY LIPS. NO BILL CIPHER JOKES.
void UCharacterGravityComponent::CustomGravityFall(float DeltaTime, FRotator newRotation, int32 Iterations) {

	// Taken mostly from CharacterMovementComponent.cpp's PhysFalling:
	float remainingTime = DeltaTime;
	while (remainingTime >= MIN_TICK_TIME && Iterations < MaxSimulationIterations) {
		Iterations++;
		float timeTick = GetSimulationTimeStep(remainingTime, Iterations);
		remainingTime -= timeTick;

		const FVector OldLocation = UpdatedComponent->GetComponentLocation();
		bJustTeleported = false;

		const FVector OldVelocityWithRootMotion = Velocity;

		float GravityTime = timeTick;

		Velocity = NewFallVelocity(Velocity, internalGravity * 100.0f, GravityTime);

		// Compute change in position (using midpoint integration method).
		FVector Adjusted = 0.5f * (OldVelocityWithRootMotion + Velocity) * timeTick;

		FHitResult Hit(1.f);
		SafeMoveUpdatedComponent(Adjusted, newRotation, true, Hit);

		float subTimeTickRemaining = timeTick * (1.f - Hit.Time);
		if (Hit.bBlockingHit) {
			if (Hit.Normal.Dot(-internalGravity.GetSafeNormal()) > 0.5) {
				remainingTime += subTimeTickRemaining;
				SetMovementMode(EMovementMode::MOVE_Custom, CUSTOM_GRAVITY_WALK);
				return;
			}
		}
	}
}

void UCharacterGravityComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) {
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (MovementMode.GetValue() != EMovementMode::MOVE_Custom) {
		AddForce(internalGravity * 10000.0f);
		FRotator newRotation;
		if (RotateTowardsGravity(DeltaTime, newRotation)) {
			FHitResult Adjustment(1.f);
			SafeMoveUpdatedComponent(FVector::ZeroVector, newRotation, false, Adjustment);
		}
	}
}

void UCharacterGravityComponent::SetIsSkiing(bool isSkiing) {
	if (isSkiing != bIsSkiing) {
		if (isSkiing) {
			preSkiingMovementMode = MovementMode;
			preSkiingCustomMovementMode = (GravityMovementMode)CustomMovementMode;
			if (MovementMode != EMovementMode::MOVE_Custom) {
				SetMovementMode(EMovementMode::MOVE_Custom, GravityMovementMode::CUSTOM_GRAVITY_WALK);
			}
		}
		else {
			SetMovementMode(preSkiingMovementMode, preSkiingCustomMovementMode);
		}
	}
	bIsSkiing = isSkiing;
}

// For applying forces once they've been set up:
void UCharacterGravityComponent::PhysCustom(float DeltaTime, int32 Iterations) {
	Super::PhysCustom(DeltaTime, Iterations);

	FRotator newRotation = GetLastUpdateRotation();
	RotateTowardsGravity(DeltaTime, newRotation);

	switch (CustomMovementMode) {
		case CUSTOM_GRAVITY_FALL:
			CustomGravityFall(DeltaTime, newRotation, Iterations);
		break;
		case CUSTOM_GRAVITY_WALK:
			CustomGravityWalk(DeltaTime, newRotation);
		break;
	}
}