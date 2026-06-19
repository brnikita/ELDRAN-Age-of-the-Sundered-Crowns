// Copyright KELDRAN. Replicated world state (Docs/00 §1).
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "KeldranGameState.generated.h"

UCLASS()
class KELDRANCORE_API AKeldranGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	AKeldranGameState();

	/** Zone this shard hosts (e.g. "zone.vaels-rest"). */
	UPROPERTY(ReplicatedUsing = OnRep_ZoneId, BlueprintReadOnly, Category = "Keldran")
	FName ZoneId;

	UFUNCTION()
	void OnRep_ZoneId();

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
};
