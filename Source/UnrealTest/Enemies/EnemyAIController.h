// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AISenseConfig_Sight.h"
#include "GameFramework/Actor.h"
#include "EnemyAIController.generated.h"

/**
 * 
 */
UCLASS()
class UNREALTEST_API AEnemyAIController : public AAIController
{
	GENERATED_BODY()

	AEnemyAIController();
public:
	virtual void SetGenericTeamId(const FGenericTeamId& _TeamID) override { TeamID = _TeamID; }

	virtual FGenericTeamId GetGenericTeamId() const override { return TeamID; }
	
	virtual ETeamAttitude::Type GetTeamAttitudeTowards(const AActor& Other) const override { return GetAttitudeTowards(&Other); };

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	ETeamAttitude::Type GetAttitudeTowards(const AActor* Other) const;


private:
	UPROPERTY(Category=Gameplay, EditAnywhere, BlueprintReadWrite, meta=(AllowPrivateAccess="true"))
	FGenericTeamId TeamID;

	TObjectPtr<UAIPerceptionComponent> AIPerception;
	TObjectPtr<UAISenseConfig_Sight> SightConfig;
};
