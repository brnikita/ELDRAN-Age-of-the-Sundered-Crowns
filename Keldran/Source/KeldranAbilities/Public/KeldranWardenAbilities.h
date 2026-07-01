// Copyright KELDRAN. The slice's Warden ability set (Docs/06 §4.1, Docs/00 §3). Effect classes
// (damage/stun/buff GEs) are assigned in data/BP (M2-17); logic is here.
#pragma once

#include "CoreMinimal.h"
#include "KeldranGameplayAbility.h"
#include "KeldranWardenAbilities.generated.h"

class UGameplayEffect;

/** Melee swing: applies DamageEffect to the nearest target in front. */
UCLASS()
class KELDRANABILITIES_API UGA_BasicAttack : public UKeldranGameplayAbility
{
	GENERATED_BODY()
public:
	UGA_BasicAttack();
	UPROPERTY(EditDefaultsOnly, Category = "Keldran|Ability") TSubclassOf<UGameplayEffect> DamageEffect;
	UPROPERTY(EditDefaultsOnly, Category = "Keldran|Ability") float BaseDamage = 15.f;
	UPROPERTY(EditDefaultsOnly, Category = "Keldran|Ability") float Range = 200.f;

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;
};

/** Shield bash: damage + stun (StunEffect grants State.Stunned). */
UCLASS()
class KELDRANABILITIES_API UGA_ShieldBash : public UKeldranGameplayAbility
{
	GENERATED_BODY()
public:
	UGA_ShieldBash();
	UPROPERTY(EditDefaultsOnly, Category = "Keldran|Ability") TSubclassOf<UGameplayEffect> DamageEffect;
	UPROPERTY(EditDefaultsOnly, Category = "Keldran|Ability") TSubclassOf<UGameplayEffect> StunEffect;
	UPROPERTY(EditDefaultsOnly, Category = "Keldran|Ability") float BaseDamage = 8.f;
	UPROPERTY(EditDefaultsOnly, Category = "Keldran|Ability") float Range = 200.f;

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;
};

/** Defensive stance: self-buff (BuffEffect, e.g. +Armor for a duration). */
UCLASS()
class KELDRANABILITIES_API UGA_DefensiveStance : public UKeldranGameplayAbility
{
	GENERATED_BODY()
public:
	UGA_DefensiveStance();
	UPROPERTY(EditDefaultsOnly, Category = "Keldran|Ability") TSubclassOf<UGameplayEffect> BuffEffect;

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;
};
