// Copyright KELDRAN.
#include "KeldranQuestComponent.h"
#include "Engine/DataTable.h"
#include "Net/UnrealNetwork.h"
#include "KeldranInventoryComponent.h"
#include "KeldranPlayerState.h"

UKeldranQuestComponent::UKeldranQuestComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

const FQuestRow* UKeldranQuestComponent::GetRow(FName QuestRow) const
{
	return QuestTable ? QuestTable->FindRow<FQuestRow>(QuestRow, TEXT("Quest"), false) : nullptr;
}

FQuestProgress* UKeldranQuestComponent::FindQuest(FName QuestRow)
{
	return ActiveQuests.FindByPredicate([QuestRow](const FQuestProgress& P) { return P.QuestRow == QuestRow; });
}

bool UKeldranQuestComponent::HasQuest(FName QuestRow) const
{
	return ActiveQuests.ContainsByPredicate([QuestRow](const FQuestProgress& P) { return P.QuestRow == QuestRow; });
}

EQuestStatus UKeldranQuestComponent::GetQuestStatus(FName QuestRow) const
{
	for (const FQuestProgress& P : ActiveQuests)
	{
		if (P.QuestRow == QuestRow) { return P.Status; }
	}
	return EQuestStatus::Active; // caller should check HasQuest first
}

bool UKeldranQuestComponent::AcceptQuest(FName QuestRow)
{
	const FQuestRow* Row = GetRow(QuestRow);
	if (!Row || HasQuest(QuestRow))
	{
		return false;
	}
	// Prereqs must be turned in.
	for (const FName& Prereq : Row->PrereqQuests)
	{
		const FQuestProgress* P = const_cast<UKeldranQuestComponent*>(this)->FindQuest(Prereq);
		if (!P || P->Status != EQuestStatus::TurnedIn)
		{
			return false;
		}
	}

	FQuestProgress Progress;
	Progress.QuestRow = QuestRow;
	Progress.Status = EQuestStatus::Active;
	Progress.ObjectiveProgress.Init(0, Row->Objectives.Num());
	ActiveQuests.Add(Progress);

	// A no-objective quest is immediately complete.
	RecheckCompletion(ActiveQuests.Last());
	OnQuestsUpdated.Broadcast();
	return true;
}

void UKeldranQuestComponent::AdvanceObjective(EObjectiveType Type, FName TargetId, int32 Amount)
{
	bool bChanged = false;
	for (FQuestProgress& P : ActiveQuests)
	{
		if (P.Status != EQuestStatus::Active) { continue; }
		const FQuestRow* Row = GetRow(P.QuestRow);
		if (!Row) { continue; }
		for (int32 i = 0; i < Row->Objectives.Num(); ++i)
		{
			const FQuestObjective& Obj = Row->Objectives[i];
			if (Obj.Type == Type && Obj.TargetId == TargetId && P.ObjectiveProgress.IsValidIndex(i))
			{
				if (P.ObjectiveProgress[i] < Obj.Count)
				{
					P.ObjectiveProgress[i] = FMath::Min(Obj.Count, P.ObjectiveProgress[i] + Amount);
					bChanged = true;
				}
			}
		}
		RecheckCompletion(P);
	}
	if (bChanged)
	{
		OnQuestsUpdated.Broadcast();
	}
}

void UKeldranQuestComponent::RecheckCompletion(FQuestProgress& Progress)
{
	if (Progress.Status != EQuestStatus::Active) { return; }
	const FQuestRow* Row = GetRow(Progress.QuestRow);
	if (!Row) { return; }
	for (int32 i = 0; i < Row->Objectives.Num(); ++i)
	{
		if (!Progress.ObjectiveProgress.IsValidIndex(i) || Progress.ObjectiveProgress[i] < Row->Objectives[i].Count)
		{
			return; // not all objectives met
		}
	}
	Progress.Status = EQuestStatus::Complete;
}

void UKeldranQuestComponent::NotifyKill(FName MobRow)        { AdvanceObjective(EObjectiveType::Kill, MobRow, 1); }
void UKeldranQuestComponent::NotifyCollect(FName ItemRow, int32 Amount) { AdvanceObjective(EObjectiveType::Collect, ItemRow, Amount); }
void UKeldranQuestComponent::NotifyReach(FName MarkerId)     { AdvanceObjective(EObjectiveType::Reach, MarkerId, 1); }

bool UKeldranQuestComponent::TurnInQuest(FName QuestRow)
{
	FQuestProgress* P = FindQuest(QuestRow);
	const FQuestRow* Row = GetRow(QuestRow);
	if (!P || !Row || P->Status != EQuestStatus::Complete)
	{
		return false;
	}
	GrantRewards(*Row);
	P->Status = EQuestStatus::TurnedIn;
	OnQuestsUpdated.Broadcast();
	return true;
}

void UKeldranQuestComponent::GrantRewards(const FQuestRow& Row)
{
	if (UKeldranInventoryComponent* Inv = GetOwner() ? GetOwner()->FindComponentByClass<UKeldranInventoryComponent>() : nullptr)
	{
		for (const FQuestReward& R : Row.RewardItems)
		{
			Inv->AddItem(R.ItemRow, R.Qty);
		}
	}
	if (AKeldranPlayerState* PS = Cast<AKeldranPlayerState>(GetOwner()))
	{
		PS->XP += Row.RewardXP;
	}
	// Coin credited to currency in M2-16 persistence.
}

void UKeldranQuestComponent::OnRep_Quests()
{
	OnQuestsUpdated.Broadcast();
}

void UKeldranQuestComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(UKeldranQuestComponent, ActiveQuests, COND_OwnerOnly);
}
