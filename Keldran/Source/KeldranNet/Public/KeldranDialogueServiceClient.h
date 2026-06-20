// Copyright KELDRAN. Server-side client for the NPC Dialogue Service (Docs/02 §6). The game
// client NEVER calls the LLM; only the dedicated server uses this. Always has an authored fallback.
#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "KeldranDialogueServiceClient.generated.h"

UCLASS(BlueprintType)
class KELDRANNET_API UKeldranDialogueServiceClient : public UObject
{
	GENERATED_BODY()

public:
	UKeldranDialogueServiceClient();

	/** Base URL of the dialogue service (default local dev). */
	UPROPERTY(BlueprintReadWrite, Category = "Keldran|Net")
	FString BaseUrl;

	/** POST /dialogue. On any failure, OnLine receives FallbackLine (service also self-falls-back). */
	void RequestLine(const FString& NpcId, const FString& AccountId, const FString& Utterance,
		const FString& FallbackLine, TFunction<void(const FString& Line, bool bFromService)> OnLine);
};
