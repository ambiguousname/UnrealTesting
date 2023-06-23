// Copyright Epic Games, Inc. All Rights Reserved.


#include "TP_WeaponComponent.h"
#include "UnrealTestCharacter.h"
#include "UnrealTestProjectile.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "Kismet/GameplayStatics.h"
#include "Components/DecalComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include <HitBehaviorInterface.h>

// Sets default values for this component's properties
UTP_WeaponComponent::UTP_WeaponComponent()
{
	fireTraceParams = FCollisionQueryParams();
}

void UTP_WeaponComponent::FireFromTrace(UWorld* World, FVector from, FVector forward, FVector newForward) {
	// newForward assumes that it's oriented based on (1, 0, 0) being absolute forward. We need to put it in terms of the actual forward vector.

	FVector to = forward - FVector::ForwardVector;
	to.Normalize();
	FRotator toRotation = forward.Rotation();

	FVector rotated = toRotation.RotateVector(newForward);
	rotated.Normalize();
	
	FHitResult out;
	bool hit = World->LineTraceSingleByChannel(out, from, from + rotated  * WeaponRange, ECC_WorldDynamic, fireTraceParams);

	if (hit) {
		UPrimitiveComponent* comp = out.GetComponent();
		//DrawDebugLine(World, from, out.ImpactPoint, FColor::Red, false, 5.0f);
		//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("%s %s"), *out.GetActor()->GetName(), *out.GetComponent()->GetName()));
		UPrimitiveComponent* componentHit = out.GetComponent();

		UMaterialInterface* decal = DefaultFiringDecal;

		AActor* currActor = out.GetActor();
		if (currActor != nullptr) {
			bool doesImp = currActor->GetClass()->ImplementsInterface(UHitBehaviorInterface::StaticClass());
			if (doesImp) {
				IHitBehaviorInterface::Execute_OnHit(currActor, out.ImpactPoint, WeaponStats);
			}
		}

		if (decal != nullptr) {
			
			UDecalComponent* spawnedDecal = UGameplayStatics::SpawnDecalAttached(DefaultFiringDecal, FVector::OneVector * 10.0f, componentHit, NAME_None, out.ImpactPoint, FRotator::ZeroRotator, EAttachLocation::KeepWorldPosition);
			if (spawnedDecal != nullptr) {
				spawnedDecal->SetFadeScreenSize(0.0f);
				// This only works if the Material has the Decal Lifetime Opacity value.
				spawnedDecal->SetFadeOut(13.0f, 2.0f, false);
			}
		}
		if (comp != nullptr && comp->IsSimulatingPhysics()) {
			componentHit->AddImpulseAtLocation(-out.ImpactNormal * FireForce, out.ImpactPoint);
		}
	}
}

void UTP_WeaponComponent::Fire()
{
	if (Character == nullptr || Character->GetController() == nullptr)
	{
		return;
	}

	UWorld* const World = GetWorld();
	if (World != nullptr)
	{
		APlayerController* PlayerController = Cast<APlayerController>(Character->GetController());

		APlayerCameraManager* camera = PlayerController->PlayerCameraManager;

		const FVector cameraPos = camera->GetCameraLocation();

		const FVector forward = camera->GetActorForwardVector();

		//Set Spawn Collision Handling Override
		FActorSpawnParameters ActorSpawnParams;
		ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

		TArray<FVector> spreadVectors = GetBulletSpread();
		for (int i = 0; i < spreadVectors.Num(); i++) {
			FVector vector = spreadVectors[i];
			FireFromTrace(World, cameraPos, forward, vector);
		}
	}
	
	// Try and play the sound if specified
	if (FireSound != nullptr)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, Character->GetActorLocation());
	}
	
	// Try and play a firing animation if specified
	if (FireAnimation != nullptr)
	{
		// Get the animation object for the arms mesh
		UAnimInstance* AnimInstance = Character->GetMesh1P()->GetAnimInstance();
		if (AnimInstance != nullptr)
		{
			AnimInstance->Montage_Play(FireAnimation, 1.f);
		}
	}
}

void UTP_WeaponComponent::AttachWeapon(AUnrealTestCharacter* TargetCharacter)
{
	Character = TargetCharacter;
	if (Character == nullptr)
	{
		return;
	}

	fireTraceParams.ClearIgnoredActors();
	fireTraceParams.AddIgnoredActor(Character);
	// Attach the weapon to the First Person Character
	FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, true);
	AttachToComponent(Character->GetMesh1P(), AttachmentRules, FName(TEXT("GripPoint")));
	
	// switch bHasRifle so the animation blueprint can switch to another animation set
	Character->SetHasRifle(true);

	// Set up action bindings
	if (APlayerController* PlayerController = Cast<APlayerController>(Character->GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			// Set the priority of the mapping to 1, so that it overrides the Jump action with the Fire action when using touch input
			Subsystem->AddMappingContext(FireMappingContext, 1);
		}

		if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerController->InputComponent))
		{
			// Fire
			EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Triggered, this, &UTP_WeaponComponent::Fire);
		}
	}
}

void UTP_WeaponComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (Character == nullptr)
	{
		return;
	}

	if (APlayerController* PlayerController = Cast<APlayerController>(Character->GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->RemoveMappingContext(FireMappingContext);
		}
	}
}