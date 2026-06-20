// Copyright KELDRAN. Quest data schema (Docs/01_DataSchemas.md §8).
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "KeldranQuestTypes.generated.h"

UENUM(BlueprintType)
enum class EObjectiveType : uint8
{
	Kill,     // TargetId = mob row
	Collect,  // TargetId = item row
	Reach     // TargetId = area marker tag
};

USTRUCT(BlueprintType)
struct FQuestObjective
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite) EObjectiveType Type = EObjectiveType::Kill;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FName TargetId;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Count = 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FText Description;
};

USTRUCT(BlueprintType)
struct FQuestReward
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite) FName ItemRow;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Qty = 1;
};

/** DT_Quests row. */
USTRUCT(BlueprintType)
struct FQuestRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite) FText DisplayName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FText SummaryText;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FName GiverNPC;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FName TurnInNPC;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FQuestObjective> Objectives;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FName> PrereqQuests;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FQuestReward> RewardItems;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 RewardCoin = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 RewardXP = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 MinLevel = 1;
};

UENUM(BlueprintType)
enum class EQuestStatus : uint8
{
	Active,
	Complete,   // all objectives met, awaiting turn-in
	TurnedIn
};

/** Live progress for one accepted quest (replicated to the owning player). */
USTRUCT(BlueprintType)
struct FQuestProgress
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly) FName QuestRow;
	UPROPERTY(BlueprintReadOnly) EQuestStatus Status = EQuestStatus::Active;
	/** Counts parallel to the quest's Objectives array. */
	UPROPERTY(BlueprintReadOnly) TArray<int32> ObjectiveProgress;
};
