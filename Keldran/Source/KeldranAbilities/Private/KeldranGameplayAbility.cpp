// Copyright KELDRAN.
#include "KeldranGameplayAbility.h"

UKeldranGameplayAbility::UKeldranGameplayAbility()
{
	// Instanced per actor: standard for MMO abilities that hold per-activation state.
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	// Server activates; client predicts where supported.
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}
