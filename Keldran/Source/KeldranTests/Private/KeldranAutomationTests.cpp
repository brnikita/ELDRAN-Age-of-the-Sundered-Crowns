// Copyright KELDRAN. Automation tests (Docs/07_TestPlan.md). Compile in any target; run headless
// via: UnrealEditor-Cmd Keldran.uproject -unattended -nullrhi -nosound
//        -ExecCmds="Automation RunTests Keldran;quit" -TestExit="Automation Test Queue Empty"
#include "Misc/AutomationTest.h"

// --- Keldran.Smoke.Basic: proves the automation harness is wired and discoverable. ---
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKeldranSmokeTest, "Keldran.Smoke.Basic",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKeldranSmokeTest::RunTest(const FString& Parameters)
{
	TestTrue(TEXT("automation harness runs"), true);
	return true;
}

// --- Keldran.Content.Originality: no banned (Tolkien/Blizzard/WoW) terms leak into content. ---
// Mirrors the build-time lint in Tools/gen/common.py. Populated against DataTable display text
// in M2-17; for now it verifies the banned-term matcher itself works.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKeldranOriginalityTest, "Keldran.Content.Originality",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKeldranOriginalityTest::RunTest(const FString& Parameters)
{
	static const TArray<FString> Banned = {
		TEXT("azeroth"), TEXT("middle-earth"), TEXT("warcraft"), TEXT("mordor"),
		TEXT("gandalf"), TEXT("stormwind"), TEXT("lich king")
	};
	auto HasBanned = [](const FString& In) -> bool
	{
		const FString Low = In.ToLower();
		for (const FString& B : Banned)
		{
			if (Low.Contains(B)) { return true; }
		}
		return false;
	};

	TestTrue(TEXT("matcher flags a banned term"), HasBanned(TEXT("The road to Azeroth")));
	TestFalse(TEXT("matcher passes original lore"), HasBanned(TEXT("Vael's Rest, near the Rift")));
	// TODO(M2-17): iterate all DT_* display text + Docs/gen manifests and assert none match.
	return true;
}

// --- Keldran.Data.Integrity: scaffold. In M2-17 this iterates every DataTable row and asserts
//     each soft asset/cross-table reference resolves (no missing icon/ability/mesh). ---
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKeldranDataIntegrityTest, "Keldran.Data.Integrity",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKeldranDataIntegrityTest::RunTest(const FString& Parameters)
{
	// No shipping DataTables exist yet (authored in M2-17). Passes vacuously; the row/asset
	// validation logic is added when DT_Items/Abilities/Mobs/Quests/etc. land.
	AddInfo(TEXT("No DataTables to validate yet (populated in M2-17)."));
	return true;
}
