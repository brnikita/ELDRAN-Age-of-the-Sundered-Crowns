// Copyright KELDRAN. Bark data schema (Docs/01_DataSchemas.md §12). Random ambient/combat lines
// for creatures and NPCs, optionally with VO (ElevenLabs, wired in M3).
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "Sound/SoundBase.h"
#include "KeldranBarkTypes.generated.h"

/** DT_Barks row. */
USTRUCT(BlueprintType)
struct FBarkRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FText> Lines;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<TSoftObjectPtr<USoundBase>> VOAssets;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FGameplayTag Context; // Bark.Aggro/Death/Idle/Greeting
};
