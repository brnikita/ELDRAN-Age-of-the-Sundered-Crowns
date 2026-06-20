// Copyright KELDRAN.
#include "KeldranPlayerState.h"
#include "Net/UnrealNetwork.h"
#include "AbilitySystemComponent.h"

AKeldranPlayerState::AKeldranPlayerState()
{
	CharacterLevel = 1;
	XP = 0;
	SetNetUpdateFrequency(10.0f);

	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed); // player: Mixed
}

void AKeldranPlayerState::OnRep_Level()
{
}

void AKeldranPlayerState::OnRep_XP()
{
}

void AKeldranPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AKeldranPlayerState, CharacterLevel);
	DOREPLIFETIME(AKeldranPlayerState, XP);
}
