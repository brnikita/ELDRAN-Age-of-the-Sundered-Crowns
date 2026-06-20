// Copyright KELDRAN.
#include "KeldranInventoryComponent.h"
#include "KeldranItemTypes.h"
#include "Engine/DataTable.h"
#include "Net/UnrealNetwork.h"

UKeldranInventoryComponent::UKeldranInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

int32 UKeldranInventoryComponent::GetMaxStack(FName ItemRow) const
{
	if (ItemTable)
	{
		if (const FItemRow* Row = ItemTable->FindRow<FItemRow>(ItemRow, TEXT("GetMaxStack"), false))
		{
			return FMath::Max(1, Row->MaxStack);
		}
	}
	return 1; // unknown item -> non-stackable (safe default until DT_Items lands, M2-17)
}

int32 UKeldranInventoryComponent::AddItem(FName ItemRow, int32 Qty)
{
	if (Qty <= 0 || ItemRow.IsNone())
	{
		return 0;
	}
	const int32 MaxStack = GetMaxStack(ItemRow);
	int32 Remaining = Qty;

	// Fill existing stacks first.
	for (FKeldranInventoryItem& It : Items)
	{
		if (Remaining <= 0) break;
		if (It.ItemRow == ItemRow && It.Qty < MaxStack)
		{
			const int32 Space = MaxStack - It.Qty;
			const int32 Add = FMath::Min(Space, Remaining);
			It.Qty += Add;
			Remaining -= Add;
		}
	}
	// New stacks for the rest.
	while (Remaining > 0)
	{
		FKeldranInventoryItem New;
		New.ItemRow = ItemRow;
		New.Qty = FMath::Min(MaxStack, Remaining);
		New.InstanceId = FGuid::NewGuid();
		New.Slot = Items.Num();
		Items.Add(New);
		Remaining -= New.Qty;
	}

	const int32 Added = Qty - Remaining;
	if (Added > 0)
	{
		OnInventoryChanged.Broadcast();
	}
	return Added;
}

int32 UKeldranInventoryComponent::RemoveItem(FName ItemRow, int32 Qty)
{
	if (Qty <= 0)
	{
		return 0;
	}
	int32 Remaining = Qty;
	for (int32 i = Items.Num() - 1; i >= 0 && Remaining > 0; --i)
	{
		if (Items[i].ItemRow == ItemRow)
		{
			const int32 Take = FMath::Min(Items[i].Qty, Remaining);
			Items[i].Qty -= Take;
			Remaining -= Take;
			if (Items[i].Qty <= 0)
			{
				Items.RemoveAt(i);
			}
		}
	}
	const int32 Removed = Qty - Remaining;
	if (Removed > 0)
	{
		OnInventoryChanged.Broadcast();
	}
	return Removed;
}

int32 UKeldranInventoryComponent::GetItemCount(FName ItemRow) const
{
	int32 Total = 0;
	for (const FKeldranInventoryItem& It : Items)
	{
		if (It.ItemRow == ItemRow)
		{
			Total += It.Qty;
		}
	}
	return Total;
}

void UKeldranInventoryComponent::ServerRequestUseItem_Implementation(FGuid InstanceId)
{
	// M2-14/M2: resolve UseEffect from DT_Items and apply via GAS, then decrement. Stubbed.
	for (const FKeldranInventoryItem& It : Items)
	{
		if (It.InstanceId == InstanceId)
		{
			UE_LOG(LogTemp, Log, TEXT("[Keldran] UseItem requested: %s"), *It.ItemRow.ToString());
			break;
		}
	}
}

void UKeldranInventoryComponent::OnRep_Items()
{
	OnInventoryChanged.Broadcast();
}

void UKeldranInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UKeldranInventoryComponent, Items);
}
