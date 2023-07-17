// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "CharacterGravityComponent.generated.h"

enum GravityMovementMode {
	// Read my lips: no Bill Cipher jokes.
	CUSTOM_GRAVITY_FALL, UMETA(DisplayName = "Custom Gravity Falling")
	CUSTOM_GRAVITY_WALK, UMETA(DisplayName = "Custom Gravity Walking")
	CUSTOM_GRAVITY_JUMP, UMETA(DisplayName = "Custom Gravity Jumping")
};

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

	FVector SkiCalcGroundVelocity(float DeltaTime);

	void SkiGroundHit(FHitResult Hit);

	void CustomGravityWalk(float DeltaTime, FRotator newRotation);

	void CustomGravityFall(float DeltaTime, FRotator newRotation, int32 Iterations);

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

// ----------------- SKIING ------------------------
public:
	UFUNCTION(BlueprintCallable, Category=Skiing)
	void SetIsSkiing(bool shouldSki);
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float skiingGroundInputFactor = 0.01f;
private:
	UPROPERTY(VisibleAnywhere, Category = Skiing)
	bool bIsSkiing = false;

	EMovementMode preSkiingMovementMode;
	GravityMovementMode preSkiingCustomMovementMode;
};
