// Copyright KELDRAN.
#include "KeldranGameModeBase.h"
#include "KeldranGameState.h"
#include "KeldranPlayerState.h"

AKeldranGameModeBase::AKeldranGameModeBase()
{
	GameStateClass = AKeldranGameState::StaticClass();
	PlayerStateClass = AKeldranPlayerState::StaticClass();
	bUseSeamlessTravel = true;
}
