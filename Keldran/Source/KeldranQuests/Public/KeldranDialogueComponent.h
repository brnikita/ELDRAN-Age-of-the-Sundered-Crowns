// Copyright KELDRAN. NPC dialogue (Docs/00 §5, Docs/06 §8). Authored lines for most NPCs; living
// NPCs (Doran Vale) route through the backend dialogue service with an authored fallback. The
// game client never calls the LLM — this runs server-side.
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "KeldranDialogueComponent.generated.h"

DECLARE_DYNAMIC_DELEGATE_OneParam(FKeldranLineDelegate, const FText&, Line);

UCLASS(ClassGroup = (Keldran), meta = (BlueprintSpawnableComponent))
class KELDRANQUESTS_API UKeldranDialogueComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UKeldranDialogueComponent();

	/** NPC id (e.g. "npc.doran-vale") used as the dialogue-service persona key. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Keldran|Dialogue")
	FName NpcId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Keldran|Dialogue")
	bool bIsLivingNPC = false;

	/** Authored lines: greeting (index 0) + fallbacks for living NPCs. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Keldran|Dialogue")
	TArray<FText> AuthoredLines;

	/** Server: produce a line for the player's utterance. Living NPCs hit the service (fallback to
	 *  authored on failure); others return an authored line. */
	void Talk(const FString& AccountId, const FString& Utterance, const FKeldranLineDelegate& OnLine);

private:
	UPROPERTY()
	TObjectPtr<class UKeldranDialogueServiceClient> ServiceClient;

	FText FirstAuthoredOr(const TCHAR* Default) const;
};
