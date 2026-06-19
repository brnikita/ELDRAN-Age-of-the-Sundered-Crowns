// Copyright KELDRAN. Base server-authoritative game mode (Docs/00 §1). Zone modes (e.g.
// AVaelsRestGameMode) derive from this in M2.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "KeldranGameModeBase.generated.h"

UCLASS()
class KELDRANCORE_API AKeldranGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	AKeldranGameModeBase();
};
