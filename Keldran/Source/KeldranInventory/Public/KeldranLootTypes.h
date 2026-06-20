// Copyright KELDRAN. Loot schema + deterministic roll (Docs/01 §7, Docs/00 §4).
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "KeldranLootTypes.generated.h"

/** One weighted drop in a loot table. */
USTRUCT(BlueprintType)
struct FLootEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite) FName ItemRow;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0")) float DropChance = 1.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 MinQty = 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 MaxQty = 1;
	/** Quest drops only roll when the looter has the relevant objective active (guaranteed then). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bIsQuestDrop = false;
};

/** DT_LootTables row. */
USTRUCT(BlueprintType)
struct FLootTableRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FLootEntry> Entries;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 GuaranteedCoinMin = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 GuaranteedCoinMax = 0;
};

/** A single rolled item drop. */
USTRUCT(BlueprintType)
struct FLootDrop
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly) FName ItemRow;
	UPROPERTY(BlueprintReadOnly) int32 Qty = 0;
};

/** Result of rolling a loot table. */
USTRUCT(BlueprintType)
struct FLootResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly) TArray<FLootDrop> Items;
	UPROPERTY(BlueprintReadOnly) int32 Coin = 0;
};
