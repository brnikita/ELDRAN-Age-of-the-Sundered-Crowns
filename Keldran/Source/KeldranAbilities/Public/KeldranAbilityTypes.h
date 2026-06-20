// Copyright KELDRAN. Ability data schema (Docs/01_DataSchemas.md §4). Drives the action bar /
// ability UI and data-assigned ability config; the runtime ability class is GA-based.
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "AttributeSet.h"
#include "KeldranAbilityTypes.generated.h"

class UGameplayAbility;

/** DT_Abilities row. */
USTRUCT(BlueprintType)
struct FAbilityRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite) FText DisplayName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FText Description;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TSoftClassPtr<UGameplayAbility> AbilityClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TSoftObjectPtr<UTexture2D> Icon;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FGameplayAttribute CostAttribute;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float CostMagnitude = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float Cooldown = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bTriggersGCD = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float CastTime = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float Range = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FGameplayTag InputTag;     // Input.Ability.SlotN
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 RequiredLevel = 1;
};
