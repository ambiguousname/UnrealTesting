// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "HitBehaviorInterface.h"
#include "Enemy.generated.h"

UCLASS()
class UNREALTEST_API AEnemy : public ACharacter, public IHitBehaviorInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AEnemy();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void OnHit_Implementation(FVector pos) override;

	UFUNCTION(BlueprintImplementableEvent)
	void OnRecieveHit();

	void OnReceiveHit_Implementation();
};
