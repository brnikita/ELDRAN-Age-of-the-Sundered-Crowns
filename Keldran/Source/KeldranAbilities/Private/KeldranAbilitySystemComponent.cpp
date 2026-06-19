// Copyright KELDRAN.
#include "KeldranAbilitySystemComponent.h"
#include "KeldranGameplayAbility.h"

void UKeldranAbilitySystemComponent::ActivateAbilityByInputTag(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid())
	{
		return;
	}
	// Find a granted KELDRAN ability bound to this input slot and try to activate it.
	for (const FGameplayAbilitySpec& Spec : GetActivatableAbilities())
	{
		const UKeldranGameplayAbility* Ability = Cast<UKeldranGameplayAbility>(Spec.Ability);
		if (Ability && Ability->InputTag == InputTag)
		{
			TryActivateAbility(Spec.Handle);
			return;
		}
	}
}
