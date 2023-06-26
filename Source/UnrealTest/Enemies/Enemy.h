// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "HitBehaviorInterface.h"
#include "UnrealTest/FP_Character/TP_WeaponComponent.h"
#include "Enemy.generated.h"

UCLASS()
class UNREALTEST_API AEnemy : public ACharacter, public IHitBehaviorInterface
{
	GENERATED_BODY()
private:
	UPROPERTY(Category=Mesh, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* WeaponMesh;

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Gameplay)
	float BaseHP = 100.0f;
protected:
	UPROPERTY(BlueprintReadOnly)
	float hp = BaseHP;
public:
	// Sets default values for this character's properties
	AEnemy();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void OnHit_Implementation(FVector pos, FWeapon weaponUsed) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	void RecieveDamage(float damage);
};
