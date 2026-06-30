// Copyright KELDRAN. Slice game mode for L1 Vael's Rest (Docs/00 §1). Sets the player pawn and
// wires per-player persistence (load on join, save on logout).
#pragma once

#include "CoreMinimal.h"
#include "KeldranGameModeBase.h"
#include "VaelsRestGameMode.generated.h"

UCLASS()
class AVaelsRestGameMode : public AKeldranGameModeBase
{
	GENERATED_BODY()

public:
	AVaelsRestGameMode();

	virtual FString InitNewPlayer(APlayerController* NewPlayerController, const FUniqueNetIdRepl& UniqueId,
		const FString& Options, const FString& Portal) override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;
};
