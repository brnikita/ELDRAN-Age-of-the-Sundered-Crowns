// Copyright KELDRAN. ASC subclass with input-tag ability activation (Docs/00 §3).
#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"
#include "KeldranAbilitySystemComponent.generated.h"

UCLASS()
class KELDRANABILITIES_API UKeldranAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	/** Activate the first grantable ability whose AbilityTags contain InputTag (server-validated). */
	void ActivateAbilityByInputTag(const FGameplayTag& InputTag);
};
