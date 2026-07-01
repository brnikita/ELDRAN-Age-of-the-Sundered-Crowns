// Copyright KELDRAN.
#include "VaelsRestGameMode.h"
#include "KeldranPlayerCharacter.h"
#include "KeldranGameSession.h"
#include "KeldranHUD.h"
#include "KeldranPlayerState.h"
#include "KeldranPersistenceSubsystem.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/GameSession.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/GameInstance.h"

AVaelsRestGameMode::AVaelsRestGameMode()
{
	DefaultPawnClass = AKeldranPlayerCharacter::StaticClass();
	GameSessionClass = AKeldranGameSession::StaticClass(); // ticket-gated logins
	HUDClass = AKeldranHUD::StaticClass();                 // spawns WBP_HUD on local clients
}

FString AVaelsRestGameMode::InitNewPlayer(APlayerController* NewPlayerController,
	const FUniqueNetIdRepl& UniqueId, const FString& Options, const FString& Portal)
{
	const FString ErrorMessage = Super::InitNewPlayer(NewPlayerController, UniqueId, Options, Portal);

	// Record the backend character id (passed as ?char=<id> on travel) for persistence.
	const FString CharId = UGameplayStatics::ParseOption(Options, TEXT("char"));
	if (NewPlayerController)
	{
		if (AKeldranPlayerState* PS = NewPlayerController->GetPlayerState<AKeldranPlayerState>())
		{
			PS->CharacterId = CharId;
		}
	}
	return ErrorMessage;
}

void AVaelsRestGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	// Load the saved snapshot for this character from the backend (server-authoritative).
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UKeldranPersistenceSubsystem* Persist = GI->GetSubsystem<UKeldranPersistenceSubsystem>())
		{
			const AKeldranPlayerState* PS = NewPlayer ? NewPlayer->GetPlayerState<AKeldranPlayerState>() : nullptr;
			if (PS && !PS->CharacterId.IsEmpty())
			{
				Persist->LoadCharacter(NewPlayer, PS->CharacterId);
			}
		}
	}
}

void AVaelsRestGameMode::Logout(AController* Exiting)
{
	// Save the character's snapshot before they leave (relog persistence).
	APlayerController* PC = Cast<APlayerController>(Exiting);
	if (PC)
	{
		if (UGameInstance* GI = GetGameInstance())
		{
			if (UKeldranPersistenceSubsystem* Persist = GI->GetSubsystem<UKeldranPersistenceSubsystem>())
			{
				const AKeldranPlayerState* PS = PC->GetPlayerState<AKeldranPlayerState>();
				if (PS && !PS->CharacterId.IsEmpty())
				{
					Persist->SaveCharacter(PC, PS->CharacterId);
				}
			}
		}
	}
	Super::Logout(Exiting);
}
