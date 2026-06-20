// Copyright KELDRAN.
#include "KeldranLootContainer.h"
#include "KeldranInventoryComponent.h"
#include "Net/UnrealNetwork.h"

AKeldranLootContainer::AKeldranLootContainer()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
}

void AKeldranLootContainer::InitContents(const FLootResult& InContents)
{
	if (!HasAuthority())
	{
		return;
	}
	Contents = InContents;
}

void AKeldranLootContainer::LootAll(AActor* Looter)
{
	if (!HasAuthority() || !Looter)
	{
		return;
	}
	UKeldranInventoryComponent* Inv = Looter->FindComponentByClass<UKeldranInventoryComponent>();
	if (!Inv)
	{
		return;
	}

	for (int32 i = Contents.Items.Num() - 1; i >= 0; --i)
	{
		const FLootDrop& Drop = Contents.Items[i];
		const int32 Added = Inv->AddItem(Drop.ItemRow, Drop.Qty);
		if (Added >= Drop.Qty)
		{
			Contents.Items.RemoveAt(i);
		}
		else
		{
			Contents.Items[i].Qty -= Added; // inventory full: keep remainder in the container
		}
	}
	// Coin would credit the looter's currency here (M2-16 persistence). Cleared for now.
	Contents.Coin = 0;

	if (Contents.Items.Num() == 0)
	{
		Destroy();
	}
}

void AKeldranLootContainer::OnRep_Contents()
{
}

void AKeldranLootContainer::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AKeldranLootContainer, Contents);
}
