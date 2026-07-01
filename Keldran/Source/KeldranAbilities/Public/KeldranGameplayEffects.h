// Copyright KELDRAN. Native GameplayEffect classes for the Warden kit (Docs/00 §3). Defined in
// code (configured in constructors) so the slice's abilities deal damage / stun / buff without
// requiring hand-authored GE assets. Designers can still subclass these as data later.
#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "KeldranGameplayEffects.generated.h"

/** Instant damage: runs KeldranDamageExecution (BaseDamage via SetByCaller + AttackPower - Armor). */
UCLASS()
class KELDRANABILITIES_API UGE_WardenDamage : public UGameplayEffect
{
	GENERATED_BODY()
public:
	UGE_WardenDamage();
};

/** 2s stun: grants State.Stunned to the target for the duration. */
UCLASS()
class KELDRANABILITIES_API UGE_WardenStun : public UGameplayEffect
{
	GENERATED_BODY()
public:
	UGE_WardenStun();
};

/** 8s defensive buff: +Armor while active. */
UCLASS()
class KELDRANABILITIES_API UGE_WardenBuffDefense : public UGameplayEffect
{
	GENERATED_BODY()
public:
	UGE_WardenBuffDefense();
};
