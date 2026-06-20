// Copyright KELDRAN. Shared player/NPC character base (Docs/00 §2). Implements the GAS interface;
// the ASC is owned by the mob itself (NPCs) or by the PlayerState (players) — derived classes set it.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "KeldranCharacterBase.generated.h"

class UAbilitySystemComponent;
class UKeldranAttributeSet;

UCLASS(Abstract)
class KELDRANCHARACTER_API AKeldranCharacterBase : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AKeldranCharacterBase();

	//~ IAbilitySystemInterface
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UKeldranAttributeSet* GetKeldranAttributeSet() const { return AttributeSet; }

	UFUNCTION(BlueprintPure, Category = "Keldran|Combat")
	bool IsDead() const;

protected:
	/** Set by derived classes (mob: own component; player: PlayerState's component). */
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY()
	TObjectPtr<UKeldranAttributeSet> AttributeSet;
};
