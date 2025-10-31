// Copyright Epic Games, Inc. All Rights Reserved.

#include "LostSectorGameMode.h"
#include "LostSectorCharacter.h"
#include "UObject/ConstructorHelpers.h"

ALostSectorGameMode::ALostSectorGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
