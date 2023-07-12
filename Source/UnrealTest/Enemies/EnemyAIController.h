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


protected:
	virtual void OnPossess(APawn* InPawn) override;

private:
	UFUNCTION()
	void TargetPerceptionUpdate(AActor* actor, FAIStimulus stimulus);

public:
	UPROPERTY(Category=AI, EditAnywhere, BlueprintReadWrite)
	UBehaviorTree* BehaviorTreeToRun;

	UPROPERTY(Category=AI, VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<AActor> sensedEnemy;

private:
	UPROPERTY(Category=Gameplay, EditAnywhere, BlueprintReadWrite, meta=(AllowPrivateAccess="true"))
	FGenericTeamId TeamID;

	TObjectPtr<UAIPerceptionComponent> AIPerception;
	TObjectPtr<UAISenseConfig_Sight> SightConfig;
};
