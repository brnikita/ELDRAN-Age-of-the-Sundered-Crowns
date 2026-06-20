// Copyright KELDRAN. Inventory stacking unit test (Docs/07 "Keldran.Inventory.StackRules").
// Builds a DataTable at runtime (no editor-only APIs) and exercises add/stack/remove logic.
#include "Misc/AutomationTest.h"
#include "Engine/DataTable.h"
#include "KeldranItemTypes.h"
#include "KeldranInventoryComponent.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKeldranInventoryStackRulesTest, "Keldran.Inventory.StackRules",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKeldranInventoryStackRulesTest::RunTest(const FString& Parameters)
{
	UDataTable* Items = NewObject<UDataTable>();
	Items->RowStruct = FItemRow::StaticStruct();
	const FString Json =
		TEXT("[")
		TEXT("{\"Name\":\"item.stackable\",\"MaxStack\":5},")
		TEXT("{\"Name\":\"item.unique\",\"MaxStack\":1}")
		TEXT("]");
	Items->CreateTableFromJSONString(Json);

	UKeldranInventoryComponent* Inv = NewObject<UKeldranInventoryComponent>();
	Inv->ItemTable = Items;

	// Add 7 of a max-5 stackable -> 7 total across 2 stacks.
	TestEqual(TEXT("added 7"), Inv->AddItem(TEXT("item.stackable"), 7), 7);
	TestEqual(TEXT("count 7"), Inv->GetItemCount(TEXT("item.stackable")), 7);

	// Add 4 more -> 11 total.
	Inv->AddItem(TEXT("item.stackable"), 4);
	TestEqual(TEXT("count 11"), Inv->GetItemCount(TEXT("item.stackable")), 11);

	// Unique items never stack: 3 -> count 3 (in 3 separate stacks).
	Inv->AddItem(TEXT("item.unique"), 3);
	TestEqual(TEXT("unique count 3"), Inv->GetItemCount(TEXT("item.unique")), 3);

	// Remove 6 -> 5 remain.
	TestEqual(TEXT("removed 6"), Inv->RemoveItem(TEXT("item.stackable"), 6), 6);
	TestEqual(TEXT("count 5"), Inv->GetItemCount(TEXT("item.stackable")), 5);

	// Over-remove clamps to what's there.
	TestEqual(TEXT("removed remaining 5"), Inv->RemoveItem(TEXT("item.stackable"), 100), 5);
	TestEqual(TEXT("count 0"), Inv->GetItemCount(TEXT("item.stackable")), 0);

	// Coin spend rules.
	Inv->AddCoin(50);
	TestFalse(TEXT("cannot overspend"), Inv->SpendCoin(60));
	TestTrue(TEXT("can spend affordable"), Inv->SpendCoin(30));
	TestEqual(TEXT("coin remaining 20"), (int32)Inv->GetCoin(), 20);

	return true;
}
