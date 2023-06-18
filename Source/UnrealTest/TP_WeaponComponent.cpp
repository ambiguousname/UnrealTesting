// Copyright Epic Games, Inc. All Rights Reserved.


#include "TP_WeaponComponent.h"
#include "UnrealTestCharacter.h"
#include "UnrealTestProjectile.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "Kismet/GameplayStatics.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

// Sets default values for this component's properties
UTP_WeaponComponent::UTP_WeaponComponent()
{
	// Default offset from the character location for projectiles to spawn
	MuzzleOffset = FVector(100.0f, 0.0f, 10.0f);
}

void UTP_WeaponComponent::BeginPlay() {
	Super::BeginPlay();
	static ConstructorHelpers::FClassFinder<UActorComponent> OnHitCompObj(TEXT("/Content/HitDecal"));
	UOnHitComponent = OnHitCompObj.Class;
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

		FHitResult out;

		bool hit = World->LineTraceSingleByChannel(out, cameraPos + forward * 100.0f, cameraPos + (forward * WeaponRange), ECC_Camera);
		if (hit) {
			UPrimitiveComponent* comp = out.GetComponent();
			//DrawDebugLine(World, cameraPos + forward * 100.0f, out.ImpactPoint, FColor::Red, false, 5.0f);
			//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("%s %s"), *out.GetActor()->GetName(), *out.GetComponent()->GetName()));
			UPrimitiveComponent* componentHit = out.GetComponent();

			UMaterialInterface* decal = DefaultFiringDecal;
			
			AActor* actor = out.GetActor();
			if (actor != nullptr) {
				actor->GetComponentByClass<UOnHitComponent>();
				FindObject<UClass>(ClassPackage, TEXT(""))
			}
			UGameplayStatics::SpawnDecalAttached(DefaultFiringDecal, FVector::OneVector * 10.0f, componentHit, NAME_None, out.ImpactPoint, FRotator::ZeroRotator, EAttachLocation::KeepWorldPosition, 15.0f);
			if (comp != nullptr && comp->IsSimulatingPhysics()) {
				componentHit->AddImpulseAtLocation(-out.ImpactNormal * FireForce, out.ImpactPoint);
			}
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