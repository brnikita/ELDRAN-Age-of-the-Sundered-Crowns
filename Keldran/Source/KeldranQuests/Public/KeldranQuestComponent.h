// Copyright KELDRAN. Per-player quest tracking (Docs/00 §5). Server-authoritative; progress
// replicates to the owning client. Other systems call NotifyKill/Collect/Reach on the server.
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "KeldranQuestTypes.h"
#include "KeldranQuestComponent.generated.h"

class UDataTable;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FKeldranQuestsUpdated);

UCLASS(ClassGroup = (Keldran), meta = (BlueprintSpawnableComponent))
class KELDRANQUESTS_API UKeldranQuestComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UKeldranQuestComponent();

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Keldran|Quests")
	TObjectPtr<UDataTable> QuestTable;

	UPROPERTY(BlueprintAssignable, Category = "Keldran|Quests")
	FKeldranQuestsUpdated OnQuestsUpdated;

	/** Server: accept a quest if prereqs + level are met and not already taken. */
	bool AcceptQuest(FName QuestRow);

	/** Server: turn in a Complete quest, granting item/coin/XP rewards. */
	bool TurnInQuest(FName QuestRow);

	/** Server: restore a quest's exact status + objective progress (used by persistence load). */
	void RestoreQuest(FName QuestRow, EQuestStatus Status, const TArray<int32>& Progress);

	// Server event hooks (called by combat / inventory / area triggers).
	void NotifyKill(FName MobRow);
	void NotifyCollect(FName ItemRow, int32 Amount);
	void NotifyReach(FName MarkerId);

	UFUNCTION(BlueprintPure, Category = "Keldran|Quests")
	EQuestStatus GetQuestStatus(FName QuestRow) const;

	UFUNCTION(BlueprintPure, Category = "Keldran|Quests")
	bool HasQuest(FName QuestRow) const;

	UFUNCTION(BlueprintPure, Category = "Keldran|Quests")
	const TArray<FQuestProgress>& GetActiveQuests() const { return ActiveQuests; }

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	UPROPERTY(ReplicatedUsing = OnRep_Quests)
	TArray<FQuestProgress> ActiveQuests;

	UFUNCTION()
	void OnRep_Quests();

	FQuestProgress* FindQuest(FName QuestRow);
	const FQuestRow* GetRow(FName QuestRow) const;
	void AdvanceObjective(EObjectiveType Type, FName TargetId, int32 Amount);
	void RecheckCompletion(FQuestProgress& Progress);
	void GrantRewards(const FQuestRow& Row);
};
