// Copyright Epic Games, Inc. All Rights Reserved.

#include "UnrealTestCharacter.h"
#include "UnrealTestProjectile.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "UnrealTest/Movement/CharacterGravityComponent.h"


//////////////////////////////////////////////////////////////////////////
// AUnrealTestCharacter

AUnrealTestCharacter::AUnrealTestCharacter(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer.SetDefaultSubobjectClass<UCharacterGravityComponent>(ACharacter::CharacterMovementComponentName))
{
	// Character doesnt have a rifle at start
	bHasRifle = false;
	
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);
		
	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-10.f, 0.f, 60.f)); // Position the camera

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	//Mesh1P->SetRelativeRotation(FRotator(0.9f, -19.19f, 5.2f));
	Mesh1P->SetRelativeLocation(FVector(-30.f, 0.f, -150.f));

}

void AUnrealTestCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	//Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

}

//////////////////////////////////////////////////////////////////////////// Input

void AUnrealTestCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		//Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		//Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AUnrealTestCharacter::Move);

		//Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AUnrealTestCharacter::Look);

		//Sliding
		EnhancedInputComponent->BindAction(SlideAction, ETriggerEvent::Triggered, this, &AUnrealTestCharacter::Slide);
	}
}

void AUnrealTestCharacter::Slide(const FInputActionValue& Value)
{
	bool isSliding = Value.Get<bool>();
	UCharacterGravityComponent* comp = Cast<UCharacterGravityComponent>(GetMovementComponent());
	if (comp) {
		if (isSliding) {
			comp->SetMovementMode(EMovementMode::MOVE_Custom, 0);
			comp->GravityShift(FVector::BackwardVector * 9.8f);
		}
		else {
			comp->GravityShift(FVector::DownVector * 9.8f);
			comp->SetMovementMode(EMovementMode::MOVE_Walking);
		}
	}
}

void AUnrealTestCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add movement 
		AddMovementInput(FirstPersonCameraComponent->GetForwardVector(), MovementVector.Y);
		AddMovementInput(FirstPersonCameraComponent->GetRightVector(), MovementVector.X);
	}
}

void AUnrealTestCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		FirstPersonCameraComponent->AddLocalRotation(FRotator(-LookAxisVector.Y, LookAxisVector.X, 0));
		FRotator cameraRot = FirstPersonCameraComponent->GetRelativeRotation();
		if (FMath::Abs(cameraRot.Pitch) > 85) {
			cameraRot.Pitch = FMath::Lerp(FMath::Sign(cameraRot.Pitch) * 85, cameraRot.Pitch, 0.1f);
		}
		// Because we're adding to local rotation, the camera can get weird about how we rotate. So we make sure to set the roll value to 0.
		FirstPersonCameraComponent->SetRelativeRotation(FRotator{ cameraRot.Pitch, cameraRot.Yaw, 0 });
	}
}

void AUnrealTestCharacter::SetHasRifle(bool bNewHasRifle)
{
	bHasRifle = bNewHasRifle;
}

bool AUnrealTestCharacter::GetHasRifle()
{
	return bHasRifle;
}