// Copyright KELDRAN.
#include "KeldranAttributeSet.h"
#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"
#include "KeldranGameplayTags.h"

UKeldranAttributeSet::UKeldranAttributeSet()
{
	InitHealth(100.f);     InitMaxHealth(100.f);
	InitMana(100.f);       InitMaxMana(100.f);
	InitStamina(100.f);    InitMaxStamina(100.f);
	InitStrength(10.f);    InitAgility(10.f);      InitIntellect(10.f);
	InitArmor(5.f);        InitMoveSpeed(600.f);
	InitAttackPower(0.f);  InitDamage(0.f);
}

void UKeldranAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxHealth());
	}
	else if (Attribute == GetManaAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxMana());
	}
	else if (Attribute == GetStaminaAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxStamina());
	}
}

void UKeldranAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	// Consume the transient Damage meta-attribute into Health (server-authoritative).
	if (Data.EvaluatedData.Attribute == GetDamageAttribute())
	{
		const float LocalDamage = GetDamage();
		SetDamage(0.f);
		if (LocalDamage > 0.f)
		{
			SetHealth(FMath::Clamp(GetHealth() - LocalDamage, 0.f, GetMaxHealth()));
		}
	}

	if (Data.EvaluatedData.Attribute == GetHealthAttribute() ||
		Data.EvaluatedData.Attribute == GetDamageAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth(), 0.f, GetMaxHealth()));
		if (GetHealth() <= 0.f)
		{
			if (UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent())
			{
				ASC->AddLooseGameplayTag(Tag_State_Dead);
			}
			OnOutOfHealth.Broadcast();
		}
	}
}

void UKeldranAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION_NOTIFY(UKeldranAttributeSet, Health,      COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UKeldranAttributeSet, MaxHealth,   COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UKeldranAttributeSet, Mana,        COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UKeldranAttributeSet, MaxMana,     COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UKeldranAttributeSet, Stamina,     COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UKeldranAttributeSet, MaxStamina,  COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UKeldranAttributeSet, Strength,    COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UKeldranAttributeSet, Agility,     COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UKeldranAttributeSet, Intellect,   COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UKeldranAttributeSet, Armor,       COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UKeldranAttributeSet, MoveSpeed,   COND_None, REPNOTIFY_Always);
}

void UKeldranAttributeSet::OnRep_Health(const FGameplayAttributeData& OldValue)      { GAMEPLAYATTRIBUTE_REPNOTIFY(UKeldranAttributeSet, Health, OldValue); }
void UKeldranAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldValue)   { GAMEPLAYATTRIBUTE_REPNOTIFY(UKeldranAttributeSet, MaxHealth, OldValue); }
void UKeldranAttributeSet::OnRep_Mana(const FGameplayAttributeData& OldValue)        { GAMEPLAYATTRIBUTE_REPNOTIFY(UKeldranAttributeSet, Mana, OldValue); }
void UKeldranAttributeSet::OnRep_MaxMana(const FGameplayAttributeData& OldValue)     { GAMEPLAYATTRIBUTE_REPNOTIFY(UKeldranAttributeSet, MaxMana, OldValue); }
void UKeldranAttributeSet::OnRep_Stamina(const FGameplayAttributeData& OldValue)     { GAMEPLAYATTRIBUTE_REPNOTIFY(UKeldranAttributeSet, Stamina, OldValue); }
void UKeldranAttributeSet::OnRep_MaxStamina(const FGameplayAttributeData& OldValue)  { GAMEPLAYATTRIBUTE_REPNOTIFY(UKeldranAttributeSet, MaxStamina, OldValue); }
void UKeldranAttributeSet::OnRep_Strength(const FGameplayAttributeData& OldValue)    { GAMEPLAYATTRIBUTE_REPNOTIFY(UKeldranAttributeSet, Strength, OldValue); }
void UKeldranAttributeSet::OnRep_Agility(const FGameplayAttributeData& OldValue)     { GAMEPLAYATTRIBUTE_REPNOTIFY(UKeldranAttributeSet, Agility, OldValue); }
void UKeldranAttributeSet::OnRep_Intellect(const FGameplayAttributeData& OldValue)   { GAMEPLAYATTRIBUTE_REPNOTIFY(UKeldranAttributeSet, Intellect, OldValue); }
void UKeldranAttributeSet::OnRep_Armor(const FGameplayAttributeData& OldValue)       { GAMEPLAYATTRIBUTE_REPNOTIFY(UKeldranAttributeSet, Armor, OldValue); }
void UKeldranAttributeSet::OnRep_MoveSpeed(const FGameplayAttributeData& OldValue)   { GAMEPLAYATTRIBUTE_REPNOTIFY(UKeldranAttributeSet, MoveSpeed, OldValue); }
