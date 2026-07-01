// Copyright KELDRAN. Slice HUD (Docs/00 §8), drawn in C++ via DrawHUD for robustness:
// live Health/Mana/Stamina bars from the owning player's GAS attributes + the action bar
// with the three generated Warden ability icons. (UMG windows arrive with M4 UI work.)
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "KeldranHUD.generated.h"

class UTexture2D;
class UAbilitySystemComponent;

UCLASS()
class KELDRANUI_API AKeldranHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
	virtual void DrawHUD() override;

private:
	UPROPERTY()
	TArray<TObjectPtr<UTexture2D>> AbilityIcons;

	UAbilitySystemComponent* GetPlayerASC() const;
	void DrawBar(float X, float Y, float W, float H, float Fraction,
		const FLinearColor& Fill, const TCHAR* Label, float Current, float Max);
	/** Floating name + health bar over each living mob within range. */
	void DrawMobNameplates();
};
