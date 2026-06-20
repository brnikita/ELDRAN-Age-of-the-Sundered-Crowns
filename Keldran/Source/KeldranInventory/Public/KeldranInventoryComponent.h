// Copyright KELDRAN. Server-authoritative inventory (Docs/00 §4, Docs/02 §3). All mutations are
// server-validated; the replicated item array drives the UI.
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "KeldranInventoryComponent.generated.h"

class UDataTable;

/** One stack/instance in an inventory. */
USTRUCT(BlueprintType)
struct FKeldranInventoryItem
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly) FName ItemRow;
	UPROPERTY(BlueprintReadOnly) int32 Qty = 1;
	UPROPERTY(BlueprintReadOnly) FGuid InstanceId;
	UPROPERTY(BlueprintReadOnly) int32 Slot = -1;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FKeldranInventoryChanged);

UCLASS(ClassGroup = (Keldran), meta = (BlueprintSpawnableComponent))
class KELDRANINVENTORY_API UKeldranInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UKeldranInventoryComponent();

	/** Source of item definitions (DT_Items). Set from data. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Keldran|Inventory")
	TObjectPtr<UDataTable> ItemTable;

	UPROPERTY(BlueprintAssignable, Category = "Keldran|Inventory")
	FKeldranInventoryChanged OnInventoryChanged;

	/** Server: add Qty of ItemRow, respecting MaxStack (stacks into existing, then new slots).
	 *  Returns the amount actually added. */
	int32 AddItem(FName ItemRow, int32 Qty);

	/** Server: remove up to Qty of ItemRow. Returns the amount actually removed. */
	int32 RemoveItem(FName ItemRow, int32 Qty);

	/** Total quantity of ItemRow across all stacks. */
	UFUNCTION(BlueprintPure, Category = "Keldran|Inventory")
	int32 GetItemCount(FName ItemRow) const;

	UFUNCTION(BlueprintPure, Category = "Keldran|Inventory")
	const TArray<FKeldranInventoryItem>& GetItems() const { return Items; }

	// --- Currency (coin) ---
	UFUNCTION(BlueprintPure, Category = "Keldran|Inventory")
	int64 GetCoin() const { return Coin; }

	/** Server: add coin. */
	void AddCoin(int64 Amount);

	/** Server: spend coin if affordable; returns true on success. */
	bool SpendCoin(int64 Amount);

	// Client requests (server-validated).
	UFUNCTION(Server, Reliable) void ServerRequestUseItem(FGuid InstanceId);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	UPROPERTY(ReplicatedUsing = OnRep_Items)
	TArray<FKeldranInventoryItem> Items;

	UPROPERTY(Replicated)
	int64 Coin = 0;

	UFUNCTION()
	void OnRep_Items();

	int32 GetMaxStack(FName ItemRow) const;
};
