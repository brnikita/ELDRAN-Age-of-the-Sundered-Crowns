// Copyright KELDRAN. Player state — holds progression; will own the AbilitySystemComponent
// once GAS comes online in M2 (Docs/00 §1, §3).
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "KeldranPlayerState.generated.h"

class UAbilitySystemComponent;

UCLASS()
class KELDRANCORE_API AKeldranPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AKeldranPlayerState();

	//~ IAbilitySystemInterface — players own their ASC on the PlayerState so it survives respawn.
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override { return AbilitySystemComponent; }

	/** Base ASC created here; the concrete AttributeSet + abilities are added by the player
	 *  character on possession (KeldranCharacter, which depends on KeldranAbilities). */
	UPROPERTY(VisibleAnywhere, Category = "Keldran|Abilities")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(ReplicatedUsing = OnRep_Level, BlueprintReadOnly, Category = "Keldran|Progression")
	int32 CharacterLevel;

	UPROPERTY(ReplicatedUsing = OnRep_XP, BlueprintReadOnly, Category = "Keldran|Progression")
	int64 XP;

	UFUNCTION()
	void OnRep_Level();

	UFUNCTION()
	void OnRep_XP();

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
};
