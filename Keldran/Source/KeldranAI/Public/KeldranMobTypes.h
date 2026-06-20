// Copyright KELDRAN. Mob data schema (Docs/01_DataSchemas.md §6).
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "KeldranMobTypes.generated.h"

class UGameplayAbility;
class UAnimInstance;
class UBehaviorTree;

/** DT_Mobs row. */
USTRUCT(BlueprintType)
struct FMobRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite) FText DisplayName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TSoftObjectPtr<USkeletalMesh> SkeletalMesh;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TSoftClassPtr<UAnimInstance> AnimBlueprint;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Level = 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bIsElite = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float Health = 50.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float AttackPower = 5.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float Armor = 2.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<TSubclassOf<UGameplayAbility>> Abilities;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TSoftObjectPtr<UBehaviorTree> BehaviorTree;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float AggroRadius = 800.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float LeashRadius = 2000.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float MovementSpeed = 400.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FName LootTable;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 XPReward = 10;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FGameplayTag Faction;
};
