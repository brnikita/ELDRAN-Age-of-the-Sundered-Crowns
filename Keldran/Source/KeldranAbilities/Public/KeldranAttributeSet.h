// Copyright KELDRAN. GAS attribute set (Docs/01_DataSchemas.md §1, Docs/00 §3).
#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "KeldranAttributeSet.generated.h"

// Standard GAS accessor boilerplate (getter/setter/initter) per attribute.
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/**
 * All KELDRAN combat attributes. Health/Mana/Stamina are clamped to their Max; reaching 0 Health
 * grants the State.Dead tag. Damage is a transient meta-attribute consumed by the damage
 * execution (Docs/00 §3, UKeldranDamageExecution in M2-3).
 */
UCLASS()
class KELDRANABILITIES_API UKeldranAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UKeldranAttributeSet();

	/** Fires on the server when Health reaches 0 after a GameplayEffect (reliable death signal). */
	FSimpleMulticastDelegate OnOutOfHealth;

	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Health, Category = "Keldran|Attributes")
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UKeldranAttributeSet, Health)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxHealth, Category = "Keldran|Attributes")
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(UKeldranAttributeSet, MaxHealth)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Mana, Category = "Keldran|Attributes")
	FGameplayAttributeData Mana;
	ATTRIBUTE_ACCESSORS(UKeldranAttributeSet, Mana)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxMana, Category = "Keldran|Attributes")
	FGameplayAttributeData MaxMana;
	ATTRIBUTE_ACCESSORS(UKeldranAttributeSet, MaxMana)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Stamina, Category = "Keldran|Attributes")
	FGameplayAttributeData Stamina;
	ATTRIBUTE_ACCESSORS(UKeldranAttributeSet, Stamina)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxStamina, Category = "Keldran|Attributes")
	FGameplayAttributeData MaxStamina;
	ATTRIBUTE_ACCESSORS(UKeldranAttributeSet, MaxStamina)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Strength, Category = "Keldran|Attributes")
	FGameplayAttributeData Strength;
	ATTRIBUTE_ACCESSORS(UKeldranAttributeSet, Strength)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Agility, Category = "Keldran|Attributes")
	FGameplayAttributeData Agility;
	ATTRIBUTE_ACCESSORS(UKeldranAttributeSet, Agility)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Intellect, Category = "Keldran|Attributes")
	FGameplayAttributeData Intellect;
	ATTRIBUTE_ACCESSORS(UKeldranAttributeSet, Intellect)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Armor, Category = "Keldran|Attributes")
	FGameplayAttributeData Armor;
	ATTRIBUTE_ACCESSORS(UKeldranAttributeSet, Armor)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MoveSpeed, Category = "Keldran|Attributes")
	FGameplayAttributeData MoveSpeed;
	ATTRIBUTE_ACCESSORS(UKeldranAttributeSet, MoveSpeed)

	// Meta attributes (server-only, transient; not replicated).
	UPROPERTY(BlueprintReadOnly, Category = "Keldran|Attributes")
	FGameplayAttributeData AttackPower;
	ATTRIBUTE_ACCESSORS(UKeldranAttributeSet, AttackPower)

	UPROPERTY(BlueprintReadOnly, Category = "Keldran|Attributes")
	FGameplayAttributeData Damage;
	ATTRIBUTE_ACCESSORS(UKeldranAttributeSet, Damage)

protected:
	UFUNCTION() void OnRep_Health(const FGameplayAttributeData& OldValue);
	UFUNCTION() void OnRep_MaxHealth(const FGameplayAttributeData& OldValue);
	UFUNCTION() void OnRep_Mana(const FGameplayAttributeData& OldValue);
	UFUNCTION() void OnRep_MaxMana(const FGameplayAttributeData& OldValue);
	UFUNCTION() void OnRep_Stamina(const FGameplayAttributeData& OldValue);
	UFUNCTION() void OnRep_MaxStamina(const FGameplayAttributeData& OldValue);
	UFUNCTION() void OnRep_Strength(const FGameplayAttributeData& OldValue);
	UFUNCTION() void OnRep_Agility(const FGameplayAttributeData& OldValue);
	UFUNCTION() void OnRep_Intellect(const FGameplayAttributeData& OldValue);
	UFUNCTION() void OnRep_Armor(const FGameplayAttributeData& OldValue);
	UFUNCTION() void OnRep_MoveSpeed(const FGameplayAttributeData& OldValue);
};
