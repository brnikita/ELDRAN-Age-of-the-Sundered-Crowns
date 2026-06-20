// Copyright KELDRAN.
#include "KeldranLootService.h"

FLootResult FKeldranLootService::RollLoot(const FLootTableRow& Table, FRandomStream& Rng, bool bIncludeQuestDrops)
{
	FLootResult Result;

	for (const FLootEntry& Entry : Table.Entries)
	{
		if (Entry.ItemRow.IsNone())
		{
			continue;
		}
		if (Entry.bIsQuestDrop)
		{
			// Quest drops are guaranteed while the objective is active; otherwise they never drop.
			if (!bIncludeQuestDrops)
			{
				continue;
			}
		}
		else
		{
			// Normal weighted roll.
			if (Rng.FRand() > Entry.DropChance)
			{
				continue;
			}
		}

		const int32 Qty = Rng.RandRange(FMath::Max(1, Entry.MinQty), FMath::Max(Entry.MinQty, Entry.MaxQty));
		FLootDrop Drop;
		Drop.ItemRow = Entry.ItemRow;
		Drop.Qty = Qty;
		Result.Items.Add(Drop);
	}

	Result.Coin = Rng.RandRange(Table.GuaranteedCoinMin, FMath::Max(Table.GuaranteedCoinMin, Table.GuaranteedCoinMax));
	return Result;
}
