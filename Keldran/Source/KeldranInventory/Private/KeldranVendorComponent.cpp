// Copyright KELDRAN.
#include "KeldranVendorComponent.h"
#include "KeldranInventoryComponent.h"
#include "KeldranItemTypes.h"

const FVendorRow* UKeldranVendorComponent::GetVendorRow() const
{
	return VendorTable ? VendorTable->FindRow<FVendorRow>(VendorRow, TEXT("Vendor"), false) : nullptr;
}

int32 UKeldranVendorComponent::GetVendorValue(FName ItemRow) const
{
	if (ItemTable)
	{
		if (const FItemRow* Row = ItemTable->FindRow<FItemRow>(ItemRow, TEXT("VendorValue"), false))
		{
			return Row->VendorValue;
		}
	}
	return 0;
}

int32 UKeldranVendorComponent::GetBuyPrice(FName ItemRow) const
{
	const FVendorRow* V = GetVendorRow();
	const float Mult = V ? V->PriceMultiplierBuy : 1.0f;
	return FMath::Max(1, FMath::RoundToInt(GetVendorValue(ItemRow) * Mult));
}

int32 UKeldranVendorComponent::GetSellPrice(FName ItemRow) const
{
	const FVendorRow* V = GetVendorRow();
	const float Mult = V ? V->PriceMultiplierSell : 0.25f;
	return FMath::Max(0, FMath::RoundToInt(GetVendorValue(ItemRow) * Mult));
}

bool UKeldranVendorComponent::BuyItem(AActor* Buyer, FName ItemRow, int32 Qty)
{
	if (!Buyer || Qty <= 0)
	{
		return false;
	}
	UKeldranInventoryComponent* Inv = Buyer->FindComponentByClass<UKeldranInventoryComponent>();
	if (!Inv)
	{
		return false;
	}
	const int64 TotalPrice = static_cast<int64>(GetBuyPrice(ItemRow)) * Qty;
	if (Inv->GetCoin() < TotalPrice)
	{
		return false;
	}
	if (!Inv->SpendCoin(TotalPrice))
	{
		return false;
	}
	Inv->AddItem(ItemRow, Qty);
	return true;
}

bool UKeldranVendorComponent::SellItem(AActor* Seller, FName ItemRow, int32 Qty)
{
	if (!Seller || Qty <= 0)
	{
		return false;
	}
	UKeldranInventoryComponent* Inv = Seller->FindComponentByClass<UKeldranInventoryComponent>();
	if (!Inv || Inv->GetItemCount(ItemRow) < Qty)
	{
		return false;
	}
	const int32 Removed = Inv->RemoveItem(ItemRow, Qty);
	if (Removed <= 0)
	{
		return false;
	}
	Inv->AddCoin(static_cast<int64>(GetSellPrice(ItemRow)) * Removed);
	return true;
}
