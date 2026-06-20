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

	// Enhanced Input (assigned in editor/data).
	UPROPERTY(EditDefaultsOnly, Category = "Keldran|Input") TObjectPtr<UInputMappingContext> InputMapping;
	UPROPERTY(EditDefaultsOnly, Category = "Keldran|Input") TObjectPtr<UInputAction> MoveAction;
	UPROPERTY(EditDefaultsOnly, Category = "Keldran|Input") TObjectPtr<UInputAction> LookAction;

protected:
	UPROPERTY(VisibleAnywhere) TObjectPtr<USpringArmComponent> SpringArm;
	UPROPERTY(VisibleAnywhere) TObjectPtr<UCameraComponent> Camera;

	bool bAbilitiesInitialized = false;

	void InitAbilityActorInfoFromPlayerState();

	void Move(const struct FInputActionValue& Value);
	void Look(const struct FInputActionValue& Value);
};
