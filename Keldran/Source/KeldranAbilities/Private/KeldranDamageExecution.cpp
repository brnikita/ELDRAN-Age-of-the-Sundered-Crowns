// Copyright KELDRAN.
#include "KeldranDamageExecution.h"
#include "KeldranAttributeSet.h"
#include "KeldranGameplayTags.h"

// Capture definitions: Armor from the target, AttackPower from the source (snapshot).
struct FKeldranDamageStatics
{
	DECLARE_ATTRIBUTE_CAPTUREDEF(Armor);
	DECLARE_ATTRIBUTE_CAPTUREDEF(AttackPower);

	FKeldranDamageStatics()
	{
		DEFINE_ATTRIBUTE_CAPTUREDEF(UKeldranAttributeSet, Armor, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UKeldranAttributeSet, AttackPower, Source, true);
	}
};

static const FKeldranDamageStatics& DamageStatics()
{
	static FKeldranDamageStatics Statics;
	return Statics;
}

UKeldranDamageExecution::UKeldranDamageExecution()
{
	RelevantAttributesToCapture.Add(DamageStatics().ArmorDef);
	RelevantAttributesToCapture.Add(DamageStatics().AttackPowerDef);
}

void UKeldranDamageExecution::Execute_Implementation(
	const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();
	FAggregatorEvaluateParameters EvalParams;
	EvalParams.SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	EvalParams.TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	float Armor = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().ArmorDef, EvalParams, Armor);

	float AttackPower = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().AttackPowerDef, EvalParams, AttackPower);

	// Base damage passed by the ability via SetByCaller(Data.Damage).
	const float BaseDamage = Spec.GetSetByCallerMagnitude(Tag_Data_Damage, false, 0.f);

	const float Final = FMath::Max(0.f, BaseDamage + AttackPower - Armor);
	if (Final > 0.f)
	{
		OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(
			UKeldranAttributeSet::GetDamageAttribute(), EGameplayModOp::Additive, Final));
	}
}
