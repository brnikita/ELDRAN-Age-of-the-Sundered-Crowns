// Copyright KELDRAN. Equipment (Docs/00 §4): equip by slot, apply the item's StatModifiers as a
// live infinite GameplayEffect, remove on unequip. Server-authoritative.
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "ActiveGameplayEffectHandle.h"
#include "KeldranEquipmentComponent.generated.h"

class UDataTable;

USTRUCT(BlueprintType)
struct FKeldranEquippedItem
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly) FGameplayTag Slot;   // Equip.Slot.*
	UPROPERTY(BlueprintReadOnly) FName ItemRow;
	UPROPERTY(BlueprintReadOnly) FGuid InstanceId;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FKeldranEquipmentChanged);

UCLASS(ClassGroup = (Keldran), meta = (BlueprintSpawnableComponent))
class KELDRANINVENTORY_API UKeldranEquipmentComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UKeldranEquipmentComponent();

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Keldran|Equipment")
	TObjectPtr<UDataTable> ItemTable;

	UPROPERTY(BlueprintAssignable, Category = "Keldran|Equipment")
	FKeldranEquipmentChanged OnEquipmentChanged;

	/** Server: equip ItemRow into its EquipSlot (replacing whatever is there) and apply its stats. */
	bool EquipItem(FName ItemRow, FGuid InstanceId);

	/** Server: unequip whatever is in Slot and remove its stat effect. */
	bool UnequipSlot(FGameplayTag Slot);

	UFUNCTION(BlueprintPure, Category = "Keldran|Equipment")
	const TArray<FKeldranEquippedItem>& GetEquipped() const { return Equipped; }

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	UPROPERTY(ReplicatedUsing = OnRep_Equipped)
	TArray<FKeldranEquippedItem> Equipped;

	UFUNCTION()
	void OnRep_Equipped();

	// Server-only: stat effect handle per slot, for removal on unequip.
	TMap<FGameplayTag, FActiveGameplayEffectHandle> ActiveStatEffects;

	class UAbilitySystemComponent* GetOwnerASC() const;
	void ApplyStatsForItem(FGameplayTag Slot, FName ItemRow);
	void RemoveStatsForSlot(FGameplayTag Slot);
};
