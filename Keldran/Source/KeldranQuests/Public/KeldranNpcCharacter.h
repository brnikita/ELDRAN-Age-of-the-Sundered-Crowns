// Copyright KELDRAN. Interactable NPC pawn (Docs/00 §5). Holds a dialogue component and the
// quests it gives; placed in the level (editor) and configured from DT_NPCs.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "KeldranNpcCharacter.generated.h"

class UKeldranDialogueComponent;
class UDataTable;

UCLASS()
class KELDRANQUESTS_API AKeldranNpcCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AKeldranNpcCharacter();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Keldran|NPC")
	TObjectPtr<UDataTable> NPCTable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Keldran|NPC")
	FName NpcRow;

	/** Quests this NPC offers (subset of DT_Quests). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Keldran|NPC")
	TArray<FName> QuestsOffered;

	UKeldranDialogueComponent* GetDialogue() const { return Dialogue; }

protected:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UKeldranDialogueComponent> Dialogue;
};
