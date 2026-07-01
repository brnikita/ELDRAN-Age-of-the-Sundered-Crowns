// Copyright KELDRAN.
#include "KeldranWardenAbilities.h"
#include "KeldranGameplayTags.h"
#include "KeldranGameplayEffects.h"

UGA_BasicAttack::UGA_BasicAttack()
{
	InputTag = Tag_Input_Ability_Slot1;
	DamageEffect = UGE_WardenDamage::StaticClass();
	ActivationSound = TSoftObjectPtr<USoundBase>(
		FSoftObjectPath(TEXT("/Game/Audio/SFX/basic_attack.basic_attack")));
}

UGA_ShieldBash::UGA_ShieldBash()
{
	InputTag = Tag_Input_Ability_Slot2;
	DamageEffect = UGE_WardenDamage::StaticClass();
	StunEffect = UGE_WardenStun::StaticClass();
	ActivationSound = TSoftObjectPtr<USoundBase>(
		FSoftObjectPath(TEXT("/Game/Audio/SFX/shield_bash.shield_bash")));
}

UGA_DefensiveStance::UGA_DefensiveStance()
{
	InputTag = Tag_Input_Ability_Slot3;
	BuffEffect = UGE_WardenBuffDefense::StaticClass();
	ActivationSound = TSoftObjectPtr<USoundBase>(
		FSoftObjectPath(TEXT("/Game/Audio/SFX/defensive_stance.defensive_stance")));
}

void UGA_BasicAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	PlayActivationSound();
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
	PlayActivationSound();
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
	PlayActivationSound();
	// Self-buff: apply to the avatar.
	ApplyEffectToActor(GetAvatarActorFromActorInfo(), BuffEffect, 0.f);
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
