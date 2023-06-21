// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HitBehaviorInterface.generated.h"


UINTERFACE(MinimalAPI)
class UHitBehaviorInterface : public UInterface
{
	GENERATED_BODY()
};

class IHitBehaviorInterface {
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Hit Behavior")
	void OnHit(FVector pos);
};