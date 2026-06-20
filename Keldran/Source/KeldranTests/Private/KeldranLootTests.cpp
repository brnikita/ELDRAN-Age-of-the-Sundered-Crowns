// Copyright KELDRAN. Loot math unit tests (Docs/07_TestPlan.md "Keldran.Loot.Tables"). Pure
// logic, no world needed — runs headless once the editor target builds.
#include "Misc/AutomationTest.h"
#include "KeldranLootTypes.h"
#include "KeldranLootService.h"

static FLootTableRow MakeTestTable()
{
	FLootTableRow T;
	// Always-drop normal item.
	{ FLootEntry E; E.ItemRow = "item.always"; E.DropChance = 1.0f; E.MinQty = 1; E.MaxQty = 1; T.Entries.Add(E); }
	// Never-drop normal item.
	{ FLootEntry E; E.ItemRow = "item.never"; E.DropChance = 0.0f; E.MinQty = 1; E.MaxQty = 1; T.Entries.Add(E); }
	// Quest drop (guaranteed only when included).
	{ FLootEntry E; E.ItemRow = "item.quest"; E.DropChance = 1.0f; E.bIsQuestDrop = true; E.MinQty = 1; E.MaxQty = 1; T.Entries.Add(E); }
	T.GuaranteedCoinMin = 5; T.GuaranteedCoinMax = 10;
	return T;
}

static bool HasItem(const FLootResult& R, FName Row)
{
	return R.Items.ContainsByPredicate([Row](const FLootDrop& D) { return D.ItemRow == Row; });
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKeldranLootTablesTest, "Keldran.Loot.Tables",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKeldranLootTablesTest::RunTest(const FString& Parameters)
{
	const FLootTableRow Table = MakeTestTable();

	// Without quest drops: always present, never absent, quest absent.
	FRandomStream Rng1(12345);
	const FLootResult R1 = FKeldranLootService::RollLoot(Table, Rng1, /*bIncludeQuestDrops=*/false);
	TestTrue(TEXT("100% item drops"), HasItem(R1, "item.always"));
	TestFalse(TEXT("0% item does not drop"), HasItem(R1, "item.never"));
	TestFalse(TEXT("quest drop absent when not included"), HasItem(R1, "item.quest"));
	TestTrue(TEXT("coin within range"), R1.Coin >= 5 && R1.Coin <= 10);

	// With quest drops: quest item guaranteed.
	FRandomStream Rng2(12345);
	const FLootResult R2 = FKeldranLootService::RollLoot(Table, Rng2, /*bIncludeQuestDrops=*/true);
	TestTrue(TEXT("quest drop guaranteed when included"), HasItem(R2, "item.quest"));

	// Determinism: same seed -> same item count + coin.
	FRandomStream RngA(999), RngB(999);
	const FLootResult A = FKeldranLootService::RollLoot(Table, RngA, true);
	const FLootResult B = FKeldranLootService::RollLoot(Table, RngB, true);
	TestEqual(TEXT("deterministic item count"), A.Items.Num(), B.Items.Num());
	TestEqual(TEXT("deterministic coin"), A.Coin, B.Coin);

	return true;
}
