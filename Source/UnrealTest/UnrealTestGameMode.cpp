// Copyright Epic Games, Inc. All Rights Reserved.

#include "UnrealTestGameMode.h"
#include "FP_Character/UnrealTestCharacter.h"
#include "UObject/ConstructorHelpers.h"

AUnrealTestGameMode::AUnrealTestGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPerson/Blueprints/BP_FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

}
