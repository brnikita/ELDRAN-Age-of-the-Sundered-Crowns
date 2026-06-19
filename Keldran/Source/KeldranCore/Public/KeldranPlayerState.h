// Copyright KELDRAN. Player state — holds progression; will own the AbilitySystemComponent
// once GAS comes online in M2 (Docs/00 §1, §3).
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "KeldranPlayerState.generated.h"

UCLASS()
class KELDRANCORE_API AKeldranPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	AKeldranPlayerState();

	UPROPERTY(ReplicatedUsing = OnRep_Level, BlueprintReadOnly, Category = "Keldran|Progression")
	int32 CharacterLevel;

	UPROPERTY(ReplicatedUsing = OnRep_XP, BlueprintReadOnly, Category = "Keldran|Progression")
	int64 XP;

	UFUNCTION()
	void OnRep_Level();

	UFUNCTION()
	void OnRep_XP();

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
};
