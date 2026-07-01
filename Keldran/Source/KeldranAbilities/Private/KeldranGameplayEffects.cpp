// Copyright KELDRAN.
#include "KeldranGameplayEffects.h"
#include "KeldranDamageExecution.h"
#include "KeldranAttributeSet.h"
#include "KeldranGameplayTags.h"
#include "GameplayEffectComponents/TargetTagsGameplayEffectComponent.h"

UGE_WardenDamage::UGE_WardenDamage()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;

	FGameplayEffectExecutionDefinition Exec;
	Exec.CalculationClass = UKeldranDamageExecution::StaticClass();
	Executions.Add(Exec);
}

UGE_WardenStun::UGE_WardenStun()
{
	DurationPolicy = EGameplayEffectDurationType::HasDuration;
	DurationMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(2.0f));

	// FindOrAddComponent() uses NewObject and is illegal in a constructor; create the component
	// as a default subobject (constructor-safe) and register it in GEComponents directly.
	UTargetTagsGameplayEffectComponent* TagComp =
		CreateDefaultSubobject<UTargetTagsGameplayEffectComponent>(TEXT("StunTargetTags"));
	FInheritedTagContainer Tags;
	Tags.AddTag(Tag_State_Stunned);
	TagComp->SetAndApplyTargetTagChanges(Tags);
	GEComponents.Add(TagComp);
}

UGE_WardenBuffDefense::UGE_WardenBuffDefense()
{
	DurationPolicy = EGameplayEffectDurationType::HasDuration;
	DurationMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(8.0f));

	FGameplayModifierInfo Mod;
	Mod.Attribute = UKeldranAttributeSet::GetArmorAttribute();
	Mod.ModifierOp = EGameplayModOp::Additive;
	Mod.ModifierMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(15.0f));
	Modifiers.Add(Mod);
}
