// Copyright KELDRAN. Deterministic loot roll (seeded for test reproducibility, Docs/07 §determinism).
#pragma once

#include "CoreMinimal.h"
#include "KeldranLootTypes.h"

class KELDRANINVENTORY_API FKeldranLootService
{
public:
	/** Roll a loot table with a seeded stream. Quest drops are included (guaranteed) only when
	 *  bIncludeQuestDrops is true (i.e. the looter has the matching objective active). */
	static FLootResult RollLoot(const FLootTableRow& Table, FRandomStream& Rng, bool bIncludeQuestDrops);
};
