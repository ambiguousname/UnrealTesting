// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/SkeletalMeshComponent.h"
#include "TP_WeaponComponent.generated.h"

class AUnrealTestCharacter;

USTRUCT(BlueprintType)
struct FWeapon {
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	float baseDamage = 10.0f;
};

UCLASS(Blueprintable, BlueprintType, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UNREALTEST_API UTP_WeaponComponent : public USkeletalMeshComponent
{
	GENERATED_BODY()
public:
	DECLARE_DELEGATE(OnHitDelegate)

public:
	UPROPERTY(EditAnywhere, Category = Firing)
	UMaterialInterface* DefaultFiringDecal;

	UPROPERTY(EditAnywhere, Category=Firing)
	float WeaponRange = 10000.0f;

	UPROPERTY(EditAnywhere, Category=Firing)
	float FireForce = 100000.0f;

	UPROPERTY(EditAnywhere, Category=Firing)
	FWeapon WeaponStats;

	/** Projectile class to spawn */
	UPROPERTY(EditDefaultsOnly, Category=Projectile)
	TSubclassOf<class AUnrealTestProjectile> ProjectileClass;

	/** Sound to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	USoundBase* FireSound;
	
	/** AnimMontage to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	UAnimMontage* FireAnimation;

	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	class UInputMappingContext* FireMappingContext;

	/** Fire Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	class UInputAction* FireAction;

public:
	/** Sets default values for this component's properties */
	UTP_WeaponComponent();

	/** Attaches the actor to a FirstPersonCharacter */
	UFUNCTION(BlueprintCallable, Category="Weapon")
	void AttachWeapon(AUnrealTestCharacter* TargetCharacter);

	/** Make the weapon Fire a Projectile */
	UFUNCTION(BlueprintCallable, Category="Weapon")
	void Fire();

	/** 
	* Get new forward vectors for where bullets should be firing to.
	* @return A list of forwards (where 1,0,0 is the default forward) to fire towards.
	*/
	UFUNCTION(BlueprintNativeEvent, Category = "Firing")
	TArray<FVector> GetBulletSpread();

	TArray<FVector> GetBulletSpread_Implementation() { auto arr = TArray<FVector>(); arr.Add(FVector::ForwardVector); return arr; }

protected:
	/** Ends gameplay for this component. */
	UFUNCTION()
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void FireFromTrace(UWorld* World, FVector from, FVector forward, FVector newForward);

private:
	/** The Character holding this weapon*/
	AUnrealTestCharacter* Character;

	FCollisionQueryParams fireTraceParams;
};