// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterGravityComponent.h"

UCharacterGravityComponent::UCharacterGravityComponent() {
	
}

void UCharacterGravityComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) {
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("E"));
}