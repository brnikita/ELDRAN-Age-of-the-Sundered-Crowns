// Copyright KELDRAN. Third-person player pawn (Docs/00 §2). ASC lives on the PlayerState; this
// pawn is the avatar. Enhanced Input assets are assigned in data (editor) — C++ binds them.
#pragma once

#include "CoreMinimal.h"
#include "KeldranCharacterBase.h"
#include "KeldranPlayerCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class UKeldranGameplayAbility;

UCLASS()
class KELDRANCHARACTER_API AKeldranPlayerCharacter : public AKeldranCharacterBase
{
	GENERATED_BODY()

public:
	AKeldranPlayerCharacter();

	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	/** Abilities granted on possession (slice: the three Warden abilities). */
	UPROPERTY(EditDefaultsOnly, Category = "Keldran|Abilities")
	TArray<TSubclassOf<UKeldranGameplayAbility>> StartingAbilities;

	// Enhanced Input. Defaults point at the generated /Game/Input assets (created by
	// Tools/gen/build_input_assets.py); overridable in a BP child or data.
	UPROPERTY(EditDefaultsOnly, Category = "Keldran|Input") TSoftObjectPtr<UInputMappingContext> InputMapping;
	UPROPERTY(EditDefaultsOnly, Category = "Keldran|Input") TSoftObjectPtr<UInputAction> MoveAction;
	UPROPERTY(EditDefaultsOnly, Category = "Keldran|Input") TSoftObjectPtr<UInputAction> LookAction;
	/** Slot1..Slot3 ability activation actions (keys 1/2/3), mapped to Input.Ability.SlotN. */
	UPROPERTY(EditDefaultsOnly, Category = "Keldran|Input") TArray<TSoftObjectPtr<UInputAction>> AbilityActions;

protected:
	UPROPERTY(VisibleAnywhere) TObjectPtr<USpringArmComponent> SpringArm;
	UPROPERTY(VisibleAnywhere) TObjectPtr<UCameraComponent> Camera;

	// Per-player gameplay components (server-authoritative state).
	UPROPERTY(VisibleAnywhere) TObjectPtr<class UKeldranInventoryComponent> Inventory;
	UPROPERTY(VisibleAnywhere) TObjectPtr<class UKeldranEquipmentComponent> Equipment;
	UPROPERTY(VisibleAnywhere) TObjectPtr<class UKeldranQuestComponent> Quests;

	bool bAbilitiesInitialized = false;

	void InitAbilityActorInfoFromPlayerState();

	void Move(const struct FInputActionValue& Value);
	void Look(const struct FInputActionValue& Value);
	void ActivateAbilitySlot(int32 SlotIndex);
	void OnAbility1() { ActivateAbilitySlot(0); }
	void OnAbility2() { ActivateAbilitySlot(1); }
	void OnAbility3() { ActivateAbilitySlot(2); }
};
