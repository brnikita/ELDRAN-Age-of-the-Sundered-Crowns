// Copyright KELDRAN. Input-wiring tests (Docs/07_TestPlan.md): verify each Warden ability's
// InputTag maps to the correct slot, and the generated IMC_Warden binds the expected keys.
#include "Misc/AutomationTest.h"
#include "KeldranWardenAbilities.h"
#include "KeldranGameplayTags.h"
#include "InputMappingContext.h"
#include "InputAction.h"

// --- Keldran.Input.AbilityTags: ability CDOs carry their slot InputTag (keys -> abilities). ---
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKeldranInputAbilityTagsTest, "Keldran.Input.AbilityTags",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKeldranInputAbilityTagsTest::RunTest(const FString& Parameters)
{
	const UGA_BasicAttack* Basic = GetDefault<UGA_BasicAttack>();
	const UGA_ShieldBash* Bash = GetDefault<UGA_ShieldBash>();
	const UGA_DefensiveStance* Stance = GetDefault<UGA_DefensiveStance>();

	TestTrue(TEXT("BasicAttack -> Slot1"), Basic->InputTag == Tag_Input_Ability_Slot1);
	TestTrue(TEXT("ShieldBash -> Slot2"), Bash->InputTag == Tag_Input_Ability_Slot2);
	TestTrue(TEXT("DefensiveStance -> Slot3"), Stance->InputTag == Tag_Input_Ability_Slot3);

	TestFalse(TEXT("BasicAttack has activation SFX"), Basic->ActivationSound.IsNull());
	TestFalse(TEXT("ShieldBash has activation SFX"), Bash->ActivationSound.IsNull());
	TestFalse(TEXT("DefensiveStance has activation SFX"), Stance->ActivationSound.IsNull());
	return true;
}

// --- Keldran.Input.MappingContext: IMC_Warden binds WASD/mouse/1-3 to the right actions. ---
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKeldranInputMappingTest, "Keldran.Input.MappingContext",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKeldranInputMappingTest::RunTest(const FString& Parameters)
{
	const UInputMappingContext* IMC = LoadObject<UInputMappingContext>(
		nullptr, TEXT("/Game/Input/IMC_Warden.IMC_Warden"));
	if (!TestNotNull(TEXT("IMC_Warden loads"), IMC))
	{
		return false;
	}
	const TArray<FEnhancedActionKeyMapping>& Mappings = IMC->GetMappings();
	TestEqual(TEXT("IMC has 8 key mappings"), Mappings.Num(), 8);

	int32 MoveKeys = 0, AbilityKeys = 0, LookKeys = 0;
	for (const FEnhancedActionKeyMapping& M : Mappings)
	{
		const FName KeyName = M.Key.GetFName();
		if (KeyName == FName("W") || KeyName == FName("A") ||
			KeyName == FName("S") || KeyName == FName("D"))
		{
			++MoveKeys;
		}
		else if (KeyName == FName("One") || KeyName == FName("Two") || KeyName == FName("Three"))
		{
			++AbilityKeys;
		}
		else if (KeyName == FName("Mouse2D"))
		{
			++LookKeys;
		}
	}
	TestEqual(TEXT("4 movement keys (WASD)"), MoveKeys, 4);
	TestEqual(TEXT("3 ability keys (1/2/3)"), AbilityKeys, 3);
	TestEqual(TEXT("1 look key (Mouse2D)"), LookKeys, 1);
	return true;
}
