// Copyright KELDRAN. Functional integration test in a real game world (headless, -nullrhi):
// the imported DataTables load, a mob spawns with a working GAS ASC, a damage GameplayEffect
// reduces health and sets the death tag, and a player spawns with its gameplay components.
// (Full mob-init + loot-on-death runs server-authoritatively in Gauntlet; loot math is unit-
// tested in Keldran.Loot.Tables.)
#include "Misc/AutomationTest.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Engine/DataTable.h"
#include "Kismet/GameplayStatics.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"

#include "KeldranMobCharacter.h"
#include "KeldranAttributeSet.h"
#include "KeldranPlayerCharacter.h"
#include "KeldranInventoryComponent.h"
#include "KeldranQuestComponent.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKeldranCombatIntegrationTest, "Keldran.Functional.CombatIntegration",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKeldranCombatIntegrationTest::RunTest(const FString& Parameters)
{
	// Imported DataTables load.
	UDataTable* Mobs = LoadObject<UDataTable>(nullptr, TEXT("/Game/Data/DT_Mobs.DT_Mobs"));
	UDataTable* Loot = LoadObject<UDataTable>(nullptr, TEXT("/Game/Data/DT_LootTables.DT_LootTables"));
	TestNotNull(TEXT("DT_Mobs imported"), Mobs);
	TestNotNull(TEXT("DT_LootTables imported"), Loot);

	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	if (!TestNotNull(TEXT("game world created"), World)) { return false; }
	FWorldContext& Ctx = GEngine->CreateNewWorldContext(EWorldType::Game);
	Ctx.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());
	World->BeginPlay();

	// Mob spawns with a working GAS ASC + attribute set.
	AKeldranMobCharacter* Mob = World->SpawnActor<AKeldranMobCharacter>(
		AKeldranMobCharacter::StaticClass(), FVector(0, 0, 100), FRotator::ZeroRotator);
	UAbilitySystemComponent* ASC = Mob ? Mob->GetAbilitySystemComponent() : nullptr;
	if (TestNotNull(TEXT("mob has ASC"), ASC))
	{
		const float Health0 = ASC->GetNumericAttribute(UKeldranAttributeSet::GetHealthAttribute());
		TestTrue(TEXT("mob starts with health > 0"), Health0 > 0.f);

		// A real instant damage GameplayEffect reduces health and (at 0) sets the death tag.
		UGameplayEffect* KillGE = NewObject<UGameplayEffect>(GetTransientPackage(), TEXT("TestKill"));
		KillGE->DurationPolicy = EGameplayEffectDurationType::Instant;
		FGameplayModifierInfo Mod;
		Mod.Attribute = UKeldranAttributeSet::GetHealthAttribute();
		Mod.ModifierOp = EGameplayModOp::Additive;
		Mod.ModifierMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(-1000.f));
		KillGE->Modifiers.Add(Mod);
		ASC->ApplyGameplayEffectToSelf(KillGE, 1.f, ASC->MakeEffectContext());

		const float Health1 = ASC->GetNumericAttribute(UKeldranAttributeSet::GetHealthAttribute());
		TestTrue(TEXT("damage GE drove health to 0"), Health1 <= 0.f);
		TestTrue(TEXT("State.Dead tag set at 0 HP"), Mob->IsDead());
	}

	// Player spawns with its gameplay components (inventory + quests).
	AKeldranPlayerCharacter* Player = World->SpawnActor<AKeldranPlayerCharacter>(
		AKeldranPlayerCharacter::StaticClass(), FVector(300, 0, 100), FRotator::ZeroRotator);
	if (TestNotNull(TEXT("player spawned"), Player))
	{
		TestNotNull(TEXT("player has inventory component"),
			Player->FindComponentByClass<UKeldranInventoryComponent>());
		TestNotNull(TEXT("player has quest component"),
			Player->FindComponentByClass<UKeldranQuestComponent>());
	}

	World->DestroyWorld(false);
	GEngine->DestroyWorldContext(World);
	return true;
}
