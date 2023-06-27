// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "CharacterGravityComponent.generated.h"

/**
 * 
 */
UCLASS()
class UNREALTEST_API UCharacterGravityComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

	UCharacterGravityComponent();
public:
	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;

	virtual void PhysCustom(float DeltaTime, int32 Iterations) override;

	void GravityShift(FVector newGravity);

	static FRotator GetRotatorFromGravity(FVector gravityDirection);
protected:
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void CustomGravityWalk();

	bool RotateTowardsGravity(float DeltaTime, FRotator& out);
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float GravityRotationRate = 1.0f;
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FVector internalGravity = FVector(0.0f, 0.0f, -9.8f);

	FVector previousGravity;

	FRotator gravityRotation;
	FRotator previousRotation;
	// Percentage:
	float gravityRotationCompletion = 0.0f;
};
