// Copyright KELDRAN.
#include "KeldranEquipmentComponent.h"
#include "KeldranItemTypes.h"
#include "Engine/DataTable.h"
#include "Net/UnrealNetwork.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "GameplayEffect.h"

UKeldranEquipmentComponent::UKeldranEquipmentComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

UAbilitySystemComponent* UKeldranEquipmentComponent::GetOwnerASC() const
{
	return UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GetOwner());
}

bool UKeldranEquipmentComponent::EquipItem(FName ItemRow, FGuid InstanceId)
{
	if (!ItemTable || ItemRow.IsNone())
	{
		return false;
	}
	const FItemRow* Row = ItemTable->FindRow<FItemRow>(ItemRow, TEXT("EquipItem"), false);
	if (!Row || !Row->EquipSlot.IsValid())
	{
		return false;
	}

	UnequipSlot(Row->EquipSlot); // clear whatever occupies the slot

	FKeldranEquippedItem Entry;
	Entry.Slot = Row->EquipSlot;
	Entry.ItemRow = ItemRow;
	Entry.InstanceId = InstanceId;
	Equipped.Add(Entry);

	ApplyStatsForItem(Row->EquipSlot, ItemRow);
	OnEquipmentChanged.Broadcast();
	return true;
}

bool UKeldranEquipmentComponent::UnequipSlot(FGameplayTag Slot)
{
	bool bRemoved = false;
	for (int32 i = Equipped.Num() - 1; i >= 0; --i)
	{
		if (Equipped[i].Slot == Slot)
		{
			Equipped.RemoveAt(i);
			bRemoved = true;
		}
	}
	if (bRemoved)
	{
		RemoveStatsForSlot(Slot);
		OnEquipmentChanged.Broadcast();
	}
	return bRemoved;
}

void UKeldranEquipmentComponent::ApplyStatsForItem(FGameplayTag Slot, FName ItemRow)
{
	UAbilitySystemComponent* ASC = GetOwnerASC();
	const FItemRow* Row = ItemTable ? ItemTable->FindRow<FItemRow>(ItemRow, TEXT("ApplyStats"), false) : nullptr;
	if (!ASC || !Row || Row->StatModifiers.Num() == 0)
	{
		return;
	}

	UGameplayEffect* GE = NewObject<UGameplayEffect>(GetTransientPackage(), FName(*FString::Printf(TEXT("Equip_%s"), *ItemRow.ToString())));
	GE->DurationPolicy = EGameplayEffectDurationType::Infinite;
	for (const FAttributeModifier& Mod : Row->StatModifiers)
	{
		FGameplayModifierInfo Info;
		Info.Attribute = Mod.Attribute;
		Info.ModifierOp = (Mod.Op == EModifierOp::Multiply) ? EGameplayModOp::Multiplicitive : EGameplayModOp::Additive;
		Info.ModifierMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(Mod.Magnitude));
		GE->Modifiers.Add(Info);
	}

	const FGameplayEffectContextHandle Ctx = ASC->MakeEffectContext();
	const FActiveGameplayEffectHandle Handle = ASC->ApplyGameplayEffectToSelf(GE, 1.f, Ctx);
	ActiveStatEffects.Add(Slot, Handle);
}

void UKeldranEquipmentComponent::RemoveStatsForSlot(FGameplayTag Slot)
{
	UAbilitySystemComponent* ASC = GetOwnerASC();
	if (FActiveGameplayEffectHandle* Handle = ActiveStatEffects.Find(Slot))
	{
		if (ASC && Handle->IsValid())
		{
			ASC->RemoveActiveGameplayEffect(*Handle);
		}
		ActiveStatEffects.Remove(Slot);
	}
}

void UKeldranEquipmentComponent::OnRep_Equipped()
{
	OnEquipmentChanged.Broadcast();
}

void UKeldranEquipmentComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UKeldranEquipmentComponent, Equipped);
}
