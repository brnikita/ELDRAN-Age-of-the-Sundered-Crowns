// Copyright KELDRAN. Server-side persistence bridge (Docs/00 §9, Docs/02 §5). Gathers a player's
// state into the gateway snapshot schema and saves it; loads + applies on login. Authoritative.
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "KeldranPersistenceSubsystem.generated.h"

class UKeldranGatewayClient;
class APlayerController;

UCLASS()
class KELDRANSERVER_API UKeldranPersistenceSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** Server: gather the player's state (level/xp/zone/pos/inventory/equipment/quests/coin)
	 *  and POST it to the gateway under CharacterId. */
	void SaveCharacter(APlayerController* PC, const FString& CharacterId);

	/** Server: load CharacterId's snapshot and apply it to the player's components/pawn. */
	void LoadCharacter(APlayerController* PC, const FString& CharacterId);

private:
	UPROPERTY()
	TObjectPtr<UKeldranGatewayClient> Gateway;

	TSharedRef<class FJsonObject> BuildSnapshot(APlayerController* PC, const FString& CharacterId) const;
};
