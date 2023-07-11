// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy.h"
#include "Engine/SkeletalMeshSocket.h"

// Sets default values
AEnemy::AEnemy()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
	if (WeaponMesh) {
		WeaponMesh->SetupAttachment(GetMesh());
		WeaponMesh->SetRelativeLocation(FVector(-71.3, -50, 134));
		WeaponMesh->SetCanEverAffectNavigation(false);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

}

// Called when the game starts or when spawned
void AEnemy::BeginPlay()
{
	Super::BeginPlay();
	hp = BaseHP;

	// The skeleton doesn't exist until play starts, so we just set up the attachment now. (Maybe PostLoad would also work?)
	FAttachmentTransformRules rules(EAttachmentRule::SnapToTarget, true);
	WeaponMesh->AttachToComponent(GetMesh(), rules, TEXT("WeaponGrip"));
}

// Called every frame
void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AEnemy::OnHit_Implementation(FVector pos, FWeapon weaponUsed) {
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("HIT"));

	RecieveDamage(weaponUsed.baseDamage);
	// If you need to access this event when it happens, look for "Add Event On Hit" in BP.
	//this->OnRecieveHit();
}

void AEnemy::RecieveDamage(float damage) {
	hp -= damage;
	if (hp <= 0) {
		Destroy();
	}
}

// Called to bind functionality to input
void AEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

