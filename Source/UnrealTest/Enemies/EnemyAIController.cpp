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
}

void AEnemyAIController::BeginPlay() {
	Super::BeginPlay();
	AIPerception->OnTargetPerceptionUpdated.AddUniqueDynamic(this, &AEnemyAIController::OnSenseUpdate);
}

void AEnemyAIController::OnSenseUpdate(AActor* actor, FAIStimulus stimulus) {
	FVector enemySensed = FVector::ZeroVector;
	if (stimulus.IsActive()) {
		enemySensed = actor->GetActorLocation();
	}
	enemySensedPos = enemySensed;
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("%i"), stimulus.IsActive()));
	UE_LOG(LogTemp, Warning, TEXT("T?"))
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