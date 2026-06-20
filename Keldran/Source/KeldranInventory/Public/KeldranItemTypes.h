// Copyright KELDRAN. Item data schema (Docs/01_DataSchemas.md §3). DataTable row + supporting types.
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "AttributeSet.h"
#include "KeldranItemTypes.generated.h"

UENUM(BlueprintType)
enum class EItemRarity : uint8
{
	Common, Uncommon, Rare, Epic
};

UENUM()
enum class EModifierOp : uint8
{
	Add, Multiply
};

/** One stat change an item applies while equipped. */
USTRUCT(BlueprintType)
struct FAttributeModifier
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite) FGameplayAttribute Attribute;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) EModifierOp Op = EModifierOp::Add;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float Magnitude = 0.f;
};

/** DT_Items row (Docs/01 §3). */
USTRUCT(BlueprintType)
struct FItemRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite) FText DisplayName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FText Description;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FGameplayTag ItemType;     // Item.Type.*
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FGameplayTag EquipSlot;    // Equip.Slot.*
	UPROPERTY(EditAnywhere, BlueprintReadWrite) EItemRarity Rarity = EItemRarity::Common;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TSoftObjectPtr<UTexture2D> Icon;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TSoftObjectPtr<USkeletalMesh> Mesh;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 MaxStack = 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FAttributeModifier> StatModifiers;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 VendorValue = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 RequiredLevel = 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FGameplayTagContainer GameplayTags;
};
