// Copyright KELDRAN.
#include "KeldranWardenAbilities.h"

void UGA_BasicAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	if (AActor* Target = FindMeleeTargetActor(Range))
	{
		ApplyEffectToActor(Target, DamageEffect, BaseDamage);
	}
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UGA_ShieldBash::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	if (AActor* Target = FindMeleeTargetActor(Range))
	{
		ApplyEffectToActor(Target, DamageEffect, BaseDamage);
		ApplyEffectToActor(Target, StunEffect, 0.f);
	}
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UGA_DefensiveStance::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	// Self-buff: apply to the avatar.
	ApplyEffectToActor(GetAvatarActorFromActorInfo(), BuffEffect, 0.f);
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
