// Copyright KELDRAN.
#include "KeldranCharacterBase.h"
#include "AbilitySystemComponent.h"
#include "KeldranAttributeSet.h"
#include "KeldranGameplayTags.h"

AKeldranCharacterBase::AKeldranCharacterBase()
{
	PrimaryActorTick.bCanEverTick = false;
}

UAbilitySystemComponent* AKeldranCharacterBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

bool AKeldranCharacterBase::IsDead() const
{
	return AbilitySystemComponent && AbilitySystemComponent->HasMatchingGameplayTag(Tag_State_Dead);
}
