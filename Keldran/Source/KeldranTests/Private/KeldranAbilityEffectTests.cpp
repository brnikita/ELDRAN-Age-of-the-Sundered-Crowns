// Copyright KELDRAN. Verifies the Warden abilities actually carry their GameplayEffects (so live
// play deals damage/stun/buff, not just SFX) and that UGE_WardenDamage reduces target health via
// the real KeldranDamageExecution path (BaseDamage via SetByCaller + AttackPower - Armor).
#include "Misc/AutomationTest.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"

#include "KeldranWardenAbilities.h"
#include "KeldranGameplayEffects.h"
#include "KeldranMobCharacter.h"
#include "KeldranAttributeSet.h"
#include "KeldranGameplayTags.h"

// --- Keldran.Ability.EffectsAssigned: ability CDOs reference their GE classes. ---
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKeldranAbilityEffectsAssignedTest, "Keldran.Ability.EffectsAssigned",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKeldranAbilityEffectsAssignedTest::RunTest(const FString& Parameters)
{
	const UGA_BasicAttack* Basic = GetDefault<UGA_BasicAttack>();
	const UGA_ShieldBash* Bash = GetDefault<UGA_ShieldBash>();
	const UGA_DefensiveStance* Stance = GetDefault<UGA_DefensiveStance>();

	TestTrue(TEXT("BasicAttack.DamageEffect = UGE_WardenDamage"),
		Basic->DamageEffect == UGE_WardenDamage::StaticClass());
	TestTrue(TEXT("ShieldBash.DamageEffect = UGE_WardenDamage"),
		Bash->DamageEffect == UGE_WardenDamage::StaticClass());
	TestTrue(TEXT("ShieldBash.StunEffect = UGE_WardenStun"),
		Bash->StunEffect == UGE_WardenStun::StaticClass());
	TestTrue(TEXT("DefensiveStance.BuffEffect = UGE_WardenBuffDefense"),
		Stance->BuffEffect == UGE_WardenBuffDefense::StaticClass());
	return true;
}

// --- Keldran.Ability.DamageApplies: the real damage GE reduces a target's health. ---
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKeldranAbilityDamageAppliesTest, "Keldran.Ability.DamageApplies",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKeldranAbilityDamageAppliesTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	if (!TestNotNull(TEXT("game world created"), World)) { return false; }
	FWorldContext& Ctx = GEngine->CreateNewWorldContext(EWorldType::Game);
	Ctx.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());
	World->BeginPlay();

	AKeldranMobCharacter* Mob = World->SpawnActor<AKeldranMobCharacter>(
		AKeldranMobCharacter::StaticClass(), FVector(0, 0, 100), FRotator::ZeroRotator);
	UAbilitySystemComponent* ASC = Mob ? Mob->GetAbilitySystemComponent() : nullptr;
	if (TestNotNull(TEXT("mob has ASC"), ASC))
	{
		const float Health0 = ASC->GetNumericAttribute(UKeldranAttributeSet::GetHealthAttribute());

		// Apply the REAL Warden damage effect with BaseDamage via SetByCaller (as the ability does).
		FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
		FGameplayEffectSpecHandle Spec =
			ASC->MakeOutgoingSpec(UGE_WardenDamage::StaticClass(), 1.f, Context);
		if (TestTrue(TEXT("damage spec valid"), Spec.IsValid()))
		{
			Spec.Data->SetSetByCallerMagnitude(Tag_Data_Damage, 15.f);
			ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());

			const float Health1 = ASC->GetNumericAttribute(UKeldranAttributeSet::GetHealthAttribute());
			TestTrue(TEXT("real damage GE reduced target health"), Health1 < Health0);
		}
	}

	World->DestroyWorld(false);
	GEngine->DestroyWorldContext(World);
	return true;
}
