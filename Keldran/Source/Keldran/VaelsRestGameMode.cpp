// Copyright KELDRAN.
#include "VaelsRestGameMode.h"
#include "KeldranPlayerCharacter.h"
#include "KeldranGameSession.h"

AVaelsRestGameMode::AVaelsRestGameMode()
{
	DefaultPawnClass = AKeldranPlayerCharacter::StaticClass();
	GameSessionClass = AKeldranGameSession::StaticClass(); // ticket-gated logins
}
