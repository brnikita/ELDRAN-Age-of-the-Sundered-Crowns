// Copyright KELDRAN. Damage execution calc (Docs/00 §3): final = max(0, Base + AttackPower - Armor).
#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "KeldranDamageExecution.generated.h"

UCLASS()
class KELDRANABILITIES_API UKeldranDamageExecution : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()

public:
	UKeldranDamageExecution();

	virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams,
		FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;
};
