// Copyright KELDRAN. Base gameplay ability (Docs/00 §3). Most abilities are data-driven
// instances configured from DT_Abilities (M2-17); concrete subclasses only for special logic.
#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GameplayTagContainer.h"
#include "KeldranGameplayAbility.generated.h"

UCLASS()
class KELDRANABILITIES_API UKeldranGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UKeldranGameplayAbility();

	/** Input slot tag (Input.Ability.SlotN) this ability binds to. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Keldran|Ability")
	FGameplayTag InputTag;
};
