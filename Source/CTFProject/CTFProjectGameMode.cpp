// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "CTFProjectGameMode.h"
#include "CTFProjectCharacter.h"
#include "UObject/ConstructorHelpers.h"

ACTFProjectGameMode::ACTFProjectGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPersonCPP/Blueprints/ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}



