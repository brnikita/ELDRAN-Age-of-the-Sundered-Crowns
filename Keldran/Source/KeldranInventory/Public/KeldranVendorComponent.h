// Copyright KELDRAN. Vendor (Docs/00 §8, Docs/01 §10). Server-authoritative buy/sell with price
// math derived from the item's VendorValue and the vendor's multipliers.
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "KeldranVendorComponent.generated.h"

class UKeldranInventoryComponent;

USTRUCT(BlueprintType)
struct FVendorEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite) FName ItemRow;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Stock = -1;        // -1 = infinite
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 RequiredLevel = 1;
};

USTRUCT(BlueprintType)
struct FVendorRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FVendorEntry> Inventory;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bBuybackEnabled = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float PriceMultiplierBuy = 1.0f;   // markup
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float PriceMultiplierSell = 0.25f; // markdown
};

UCLASS(ClassGroup = (Keldran), meta = (BlueprintSpawnableComponent))
class KELDRANINVENTORY_API UKeldranVendorComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Keldran|Vendor")
	TObjectPtr<UDataTable> ItemTable;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Keldran|Vendor")
	TObjectPtr<UDataTable> VendorTable;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Keldran|Vendor")
	FName VendorRow;

	/** Server: buyer purchases Qty of ItemRow. Returns true if the transaction completed. */
	bool BuyItem(AActor* Buyer, FName ItemRow, int32 Qty);

	/** Server: buyer sells Qty of ItemRow back. Returns true if completed. */
	bool SellItem(AActor* Seller, FName ItemRow, int32 Qty);

	/** Unit price the vendor charges to buy ItemRow (VendorValue * PriceMultiplierBuy). */
	UFUNCTION(BlueprintPure, Category = "Keldran|Vendor")
	int32 GetBuyPrice(FName ItemRow) const;

	UFUNCTION(BlueprintPure, Category = "Keldran|Vendor")
	int32 GetSellPrice(FName ItemRow) const;

protected:
	const FVendorRow* GetVendorRow() const;
	int32 GetVendorValue(FName ItemRow) const;
};
