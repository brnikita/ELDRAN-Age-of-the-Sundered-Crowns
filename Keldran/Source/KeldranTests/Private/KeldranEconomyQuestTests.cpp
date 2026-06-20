// Copyright KELDRAN. Vendor pricing + quest lifecycle unit tests (Docs/07). World-free: runtime
// DataTables + NewObject components. Run headless once the editor target builds.
#include "Misc/AutomationTest.h"
#include "Engine/DataTable.h"
#include "KeldranItemTypes.h"
#include "KeldranVendorComponent.h"
#include "KeldranQuestTypes.h"
#include "KeldranQuestComponent.h"

// ---- Keldran.Vendor.Pricing ----
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKeldranVendorPricingTest, "Keldran.Vendor.Pricing",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKeldranVendorPricingTest::RunTest(const FString& Parameters)
{
	UDataTable* Items = NewObject<UDataTable>();
	Items->RowStruct = FItemRow::StaticStruct();
	Items->CreateTableFromJSONString(TEXT("[{\"Name\":\"item.x\",\"VendorValue\":40}]"));

	UDataTable* Vendors = NewObject<UDataTable>();
	Vendors->RowStruct = FVendorRow::StaticStruct();
	Vendors->CreateTableFromJSONString(
		TEXT("[{\"Name\":\"vendor.test\",\"PriceMultiplierBuy\":1.0,\"PriceMultiplierSell\":0.25}]"));

	UKeldranVendorComponent* V = NewObject<UKeldranVendorComponent>();
	V->ItemTable = Items;
	V->VendorTable = Vendors;
	V->VendorRow = TEXT("vendor.test");

	TestEqual(TEXT("buy price = value * 1.0"), V->GetBuyPrice(TEXT("item.x")), 40);
	TestEqual(TEXT("sell price = value * 0.25"), V->GetSellPrice(TEXT("item.x")), 10);
	return true;
}

// ---- Keldran.Quests.Lifecycle ----
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKeldranQuestLifecycleTest, "Keldran.Quests.Lifecycle",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKeldranQuestLifecycleTest::RunTest(const FString& Parameters)
{
	UDataTable* Quests = NewObject<UDataTable>();
	Quests->RowStruct = FQuestRow::StaticStruct();
	Quests->CreateTableFromJSONString(
		TEXT("[{\"Name\":\"quest.test\",\"MinLevel\":1,\"Objectives\":")
		TEXT("[{\"Type\":\"Kill\",\"TargetId\":\"mob.x\",\"Count\":2}]}]"));

	UKeldranQuestComponent* Q = NewObject<UKeldranQuestComponent>();
	Q->QuestTable = Quests;

	TestTrue(TEXT("accept"), Q->AcceptQuest(TEXT("quest.test")));
	TestTrue(TEXT("has quest"), Q->HasQuest(TEXT("quest.test")));
	TestEqual(TEXT("active after accept"), Q->GetQuestStatus(TEXT("quest.test")), EQuestStatus::Active);

	Q->NotifyKill(TEXT("mob.x"));
	TestEqual(TEXT("still active after 1/2"), Q->GetQuestStatus(TEXT("quest.test")), EQuestStatus::Active);

	Q->NotifyKill(TEXT("mob.x"));
	TestEqual(TEXT("complete after 2/2"), Q->GetQuestStatus(TEXT("quest.test")), EQuestStatus::Complete);

	TestTrue(TEXT("turn in"), Q->TurnInQuest(TEXT("quest.test")));
	TestEqual(TEXT("turned in"), Q->GetQuestStatus(TEXT("quest.test")), EQuestStatus::TurnedIn);
	return true;
}
