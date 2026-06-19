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

protected:
	/** Nearest other pawn with an ASC within Range in front of the avatar (melee target). */
	AActor* FindMeleeTargetActor(float Range) const;

	/** Build EffectClass spec (with SetByCaller Data.Damage=DamageMagnitude) and apply to Target. */
	void ApplyEffectToActor(AActor* TargetActor, TSubclassOf<class UGameplayEffect> EffectClass,
		float DamageMagnitude);
};
