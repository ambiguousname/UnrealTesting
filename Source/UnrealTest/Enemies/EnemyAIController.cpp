// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyAIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "BehaviorTree/BlackboardComponent.h"

AEnemyAIController::AEnemyAIController() {
	AIPerception = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerception"));

	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("Sight Sense"));

	AIPerception->ConfigureSense(*SightConfig);
	AIPerception->SetDominantSense(SightConfig->GetSenseImplementation());
	SetPerceptionComponent(*AIPerception);
	AIPerception->OnTargetPerceptionUpdated.AddDynamic(this, &AEnemyAIController::TargetPerceptionUpdate);
}

void AEnemyAIController::TargetPerceptionUpdate(AActor* actor, FAIStimulus stimulus) {
	if (stimulus.IsActive()) {
		sensedEnemy = actor;
	}
	else {
		sensedEnemy = nullptr;
	}
}

void AEnemyAIController::OnPossess(APawn* InPawn) {
	Super::OnPossess(InPawn);
	RunBehaviorTree(BehaviorTreeToRun);
}

ETeamAttitude::Type AEnemyAIController::GetAttitudeTowards_Implementation(const AActor* Other) const {
	const APawn* pawn = Cast<APawn>(Other);
	AAIController* aiController = nullptr;
	if (pawn) {
		aiController = Cast<AAIController>(pawn->GetController());
	}
	if (aiController && aiController->GetGenericTeamId() == GetGenericTeamId()) {
		return ETeamAttitude::Friendly;
	} else {
		return ETeamAttitude::Hostile;
	}
}