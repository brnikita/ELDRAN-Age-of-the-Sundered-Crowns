// Copyright KELDRAN.
#include "KeldranPersistenceSubsystem.h"
#include "KeldranGatewayClient.h"
#include "KeldranPlayerState.h"
#include "KeldranInventoryComponent.h"
#include "KeldranEquipmentComponent.h"
#include "KeldranQuestComponent.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerState.h"

namespace
{
	template <class T>
	T* FindOnPlayer(APlayerController* PC)
	{
		if (!PC) return nullptr;
		if (APawn* P = PC->GetPawn()) { if (T* C = P->FindComponentByClass<T>()) return C; }
		if (APlayerState* PS = PC->PlayerState) { if (T* C = PS->FindComponentByClass<T>()) return C; }
		return nullptr;
	}

	const TCHAR* QuestStatusToString(EQuestStatus S)
	{
		switch (S)
		{
		case EQuestStatus::Complete:  return TEXT("complete");
		case EQuestStatus::TurnedIn:  return TEXT("turned_in");
		default:                      return TEXT("active");
		}
	}
}

void UKeldranPersistenceSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	Gateway = NewObject<UKeldranGatewayClient>(this);
}

void UKeldranPersistenceSubsystem::Deinitialize()
{
	Gateway = nullptr;
	Super::Deinitialize();
}

TSharedRef<FJsonObject> UKeldranPersistenceSubsystem::BuildSnapshot(APlayerController* PC, const FString& CharacterId) const
{
	TSharedRef<FJsonObject> Root = MakeShared<FJsonObject>();
	Root->SetStringField(TEXT("character_id"), CharacterId);

	int32 Level = 1; int64 XP = 0;
	if (AKeldranPlayerState* PS = PC ? PC->GetPlayerState<AKeldranPlayerState>() : nullptr)
	{
		Level = PS->CharacterLevel;
		XP = PS->XP;
	}
	Root->SetNumberField(TEXT("level"), Level);
	Root->SetNumberField(TEXT("xp"), static_cast<double>(XP));
	Root->SetStringField(TEXT("zone_id"), TEXT("zone.vaels-rest"));

	FVector Pos = FVector::ZeroVector; float Yaw = 0.f;
	if (APawn* Pawn = PC ? PC->GetPawn() : nullptr)
	{
		Pos = Pawn->GetActorLocation();
		Yaw = Pawn->GetActorRotation().Yaw;
	}
	TArray<TSharedPtr<FJsonValue>> PosArr;
	PosArr.Add(MakeShared<FJsonValueNumber>(Pos.X));
	PosArr.Add(MakeShared<FJsonValueNumber>(Pos.Y));
	PosArr.Add(MakeShared<FJsonValueNumber>(Pos.Z));
	Root->SetArrayField(TEXT("pos"), PosArr);
	Root->SetNumberField(TEXT("rot_yaw"), Yaw);

	// Inventory + coin
	int64 Coin = 0;
	TArray<TSharedPtr<FJsonValue>> InvArr;
	if (UKeldranInventoryComponent* Inv = FindOnPlayer<UKeldranInventoryComponent>(PC))
	{
		Coin = Inv->GetCoin();
		for (const FKeldranInventoryItem& It : Inv->GetItems())
		{
			TSharedRef<FJsonObject> O = MakeShared<FJsonObject>();
			O->SetStringField(TEXT("item_row"), It.ItemRow.ToString());
			O->SetNumberField(TEXT("qty"), It.Qty);
			O->SetStringField(TEXT("instance_id"), It.InstanceId.ToString(EGuidFormats::DigitsWithHyphens));
			O->SetNumberField(TEXT("slot"), It.Slot);
			InvArr.Add(MakeShared<FJsonValueObject>(O));
		}
	}
	Root->SetArrayField(TEXT("inventory"), InvArr);

	// Equipment
	TArray<TSharedPtr<FJsonValue>> EquipArr;
	if (UKeldranEquipmentComponent* Equip = FindOnPlayer<UKeldranEquipmentComponent>(PC))
	{
		for (const FKeldranEquippedItem& E : Equip->GetEquipped())
		{
			TSharedRef<FJsonObject> O = MakeShared<FJsonObject>();
			O->SetStringField(TEXT("slot"), E.Slot.ToString());
			O->SetStringField(TEXT("item_row"), E.ItemRow.ToString());
			O->SetStringField(TEXT("instance_id"), E.InstanceId.ToString(EGuidFormats::DigitsWithHyphens));
			EquipArr.Add(MakeShared<FJsonValueObject>(O));
		}
	}
	Root->SetArrayField(TEXT("equipment"), EquipArr);

	// Quests
	TArray<TSharedPtr<FJsonValue>> QuestArr;
	if (UKeldranQuestComponent* Q = FindOnPlayer<UKeldranQuestComponent>(PC))
	{
		for (const FQuestProgress& P : Q->GetActiveQuests())
		{
			TSharedRef<FJsonObject> O = MakeShared<FJsonObject>();
			O->SetStringField(TEXT("quest_row"), P.QuestRow.ToString());
			O->SetStringField(TEXT("status"), QuestStatusToString(P.Status));
			TArray<TSharedPtr<FJsonValue>> Prog;
			for (int32 V : P.ObjectiveProgress) { Prog.Add(MakeShared<FJsonValueNumber>(V)); }
			O->SetArrayField(TEXT("objectives"), Prog);
			QuestArr.Add(MakeShared<FJsonValueObject>(O));
		}
	}
	Root->SetArrayField(TEXT("quests"), QuestArr);

	Root->SetNumberField(TEXT("coin"), static_cast<double>(Coin));
	Root->SetNumberField(TEXT("shards"), 0);
	return Root;
}

void UKeldranPersistenceSubsystem::SaveCharacter(APlayerController* PC, const FString& CharacterId)
{
	if (!Gateway || !PC || CharacterId.IsEmpty())
	{
		return;
	}
	const TSharedRef<FJsonObject> Snapshot = BuildSnapshot(PC, CharacterId);
	Gateway->SaveCharacter(Snapshot, [CharacterId](bool bOk)
	{
		UE_LOG(LogTemp, Log, TEXT("[Keldran] SaveCharacter %s -> %s"), *CharacterId, bOk ? TEXT("ok") : TEXT("FAILED"));
	});
}

void UKeldranPersistenceSubsystem::LoadCharacter(APlayerController* PC, const FString& CharacterId)
{
	if (!Gateway || !PC || CharacterId.IsEmpty())
	{
		return;
	}
	TWeakObjectPtr<APlayerController> WeakPC(PC);
	Gateway->LoadCharacter(CharacterId, [this, WeakPC](bool bOk, TSharedPtr<FJsonObject> Snap)
	{
		APlayerController* PC2 = WeakPC.Get();
		if (!bOk || !Snap.IsValid() || !PC2)
		{
			return;
		}
		// Level / XP
		if (AKeldranPlayerState* PS = PC2->GetPlayerState<AKeldranPlayerState>())
		{
			PS->CharacterLevel = static_cast<int32>(Snap->GetNumberField(TEXT("level")));
			PS->XP = static_cast<int64>(Snap->GetNumberField(TEXT("xp")));
			UE_LOG(LogTemp, Warning, TEXT("[Keldran] LoadCharacter: level=%d xp=%lld coin=%d"),
				PS->CharacterLevel, (long long)PS->XP, (int)Snap->GetNumberField(TEXT("coin")));
		}
		// Position
		const TArray<TSharedPtr<FJsonValue>>* PosArr;
		if (Snap->TryGetArrayField(TEXT("pos"), PosArr) && PosArr->Num() == 3)
		{
			const FVector Pos((*PosArr)[0]->AsNumber(), (*PosArr)[1]->AsNumber(), (*PosArr)[2]->AsNumber());
			if (APawn* Pawn = PC2->GetPawn())
			{
				Pawn->SetActorLocation(Pos);
			}
		}
		// Coin + inventory + equipment
		if (UKeldranInventoryComponent* Inv = FindOnPlayer<UKeldranInventoryComponent>(PC2))
		{
			Inv->AddCoin(static_cast<int64>(Snap->GetNumberField(TEXT("coin"))));
			const TArray<TSharedPtr<FJsonValue>>* InvArr;
			if (Snap->TryGetArrayField(TEXT("inventory"), InvArr))
			{
				for (const TSharedPtr<FJsonValue>& V : *InvArr)
				{
					const TSharedPtr<FJsonObject> O = V->AsObject();
					if (O.IsValid())
					{
						Inv->AddItem(FName(*O->GetStringField(TEXT("item_row"))),
							static_cast<int32>(O->GetNumberField(TEXT("qty"))));
					}
				}
			}
		}
		if (UKeldranEquipmentComponent* Equip = FindOnPlayer<UKeldranEquipmentComponent>(PC2))
		{
			const TArray<TSharedPtr<FJsonValue>>* EquipArr;
			if (Snap->TryGetArrayField(TEXT("equipment"), EquipArr))
			{
				for (const TSharedPtr<FJsonValue>& V : *EquipArr)
				{
					const TSharedPtr<FJsonObject> O = V->AsObject();
					if (O.IsValid())
					{
						FGuid Id; FGuid::Parse(O->GetStringField(TEXT("instance_id")), Id);
						Equip->EquipItem(FName(*O->GetStringField(TEXT("item_row"))), Id);
					}
				}
			}
		}
		// Quests: restore exact status + objective progress.
		if (UKeldranQuestComponent* Q = FindOnPlayer<UKeldranQuestComponent>(PC2))
		{
			const TArray<TSharedPtr<FJsonValue>>* QArr;
			if (Snap->TryGetArrayField(TEXT("quests"), QArr))
			{
				for (const TSharedPtr<FJsonValue>& V : *QArr)
				{
					const TSharedPtr<FJsonObject> O = V->AsObject();
					if (!O.IsValid()) { continue; }
					const FString StatusStr = O->GetStringField(TEXT("status"));
					EQuestStatus Status = EQuestStatus::Active;
					if (StatusStr == TEXT("complete"))  { Status = EQuestStatus::Complete; }
					else if (StatusStr == TEXT("turned_in")) { Status = EQuestStatus::TurnedIn; }

					TArray<int32> Progress;
					const TArray<TSharedPtr<FJsonValue>>* ObjArr;
					if (O->TryGetArrayField(TEXT("objectives"), ObjArr))
					{
						for (const TSharedPtr<FJsonValue>& N : *ObjArr) { Progress.Add(static_cast<int32>(N->AsNumber())); }
					}
					Q->RestoreQuest(FName(*O->GetStringField(TEXT("quest_row"))), Status, Progress);
				}
			}
		}
		UE_LOG(LogTemp, Log, TEXT("[Keldran] LoadCharacter applied snapshot (inv/equip/quests/coin/pos)."));
	});
}
