// Copyright KELDRAN.
#include "KeldranGameState.h"
#include "Net/UnrealNetwork.h"

AKeldranGameState::AKeldranGameState()
{
	ZoneId = NAME_None;
}

void AKeldranGameState::OnRep_ZoneId()
{
}

void AKeldranGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AKeldranGameState, ZoneId);
}
