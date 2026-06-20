// Copyright KELDRAN. NPC data schema (Docs/01_DataSchemas.md §9).
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "KeldranNPCTypes.generated.h"

UENUM(BlueprintType)
enum class ENPCRole : uint8
{
	Flavor,
	QuestGiver,
	Vendor,
	QuestGiverVendor
};

/** DT_NPCs row. */
USTRUCT(BlueprintType)
struct FNPCRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite) FText DisplayName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FGameplayTag Race;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TSoftObjectPtr<USkeletalMesh> SkeletalMesh;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) ENPCRole Role = ENPCRole::Flavor;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FName VendorTable;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FName> Quests;
	/** Runtime "living NPC" demo only (slice: Doran Vale). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bIsLivingNPC = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FText> FallbackLines;
};
