// Copyright KELDRAN. NPC/mob pawn (Docs/00 §6). Owns its own ASC (Minimal replication),
// initialized from DT_Mobs; spawns a loot container on death.
#pragma once

#include "CoreMinimal.h"
#include "KeldranCharacterBase.h"
#include "KeldranMobCharacter.generated.h"

class UDataTable;

UCLASS()
class KELDRANAI_API AKeldranMobCharacter : public AKeldranCharacterBase
{
	GENERATED_BODY()

public:
	AKeldranMobCharacter();

	/** DT_Mobs table + row this mob is built from (set on spawn). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Keldran|Mob")
	TObjectPtr<UDataTable> MobTable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Keldran|Mob")
	FName MobRow;

	/** DT_LootTables source for the death drop. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Keldran|Mob")
	TObjectPtr<UDataTable> LootTableAsset;

	virtual void BeginPlay() override;

	/** Localized display name from DT_Mobs (for nameplates); empty until InitFromRow runs. */
	const FText& GetDisplayName() const { return CachedDisplayName; }

protected:
	void InitFromRow();
	void HandleDeath();

	void OnDeadTagChanged(const struct FGameplayTag CallbackTag, int32 NewCount);

	FText CachedDisplayName;
	FName CachedLootTable;
	int32 CachedXPReward = 0;
	bool bDeathHandled = false;
};
