// Copyright KELDRAN.
#include "KeldranHUD.h"
#include "Engine/Canvas.h"
#include "Engine/Texture2D.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/PlayerState.h"
#include "KeldranAttributeSet.h"

namespace
{
	const TCHAR* IconPaths[] = {
		TEXT("/Game/UI/Icons/Abilities/basic_attack.basic_attack"),
		TEXT("/Game/UI/Icons/Abilities/shield_bash.shield_bash"),
		TEXT("/Game/UI/Icons/Abilities/defensive_stance.defensive_stance"),
	};
}

void AKeldranHUD::BeginPlay()
{
	Super::BeginPlay();
	for (const TCHAR* Path : IconPaths)
	{
		if (UTexture2D* Tex = LoadObject<UTexture2D>(nullptr, Path))
		{
			AbilityIcons.Add(Tex);
		}
	}
	UE_LOG(LogTemp, Log, TEXT("[Keldran] HUD ready (icons loaded: %d)."), AbilityIcons.Num());
}

UAbilitySystemComponent* AKeldranHUD::GetPlayerASC() const
{
	const APlayerController* PC = GetOwningPlayerController();
	if (const IAbilitySystemInterface* ASI = PC ? Cast<IAbilitySystemInterface>(PC->PlayerState) : nullptr)
	{
		return ASI->GetAbilitySystemComponent();
	}
	return nullptr;
}

void AKeldranHUD::DrawBar(float X, float Y, float W, float H, float Fraction,
	const FLinearColor& Fill, const TCHAR* Label, float Current, float Max)
{
	DrawRect(FLinearColor(0.f, 0.f, 0.f, 0.55f), X - 2, Y - 2, W + 4, H + 4); // frame
	DrawRect(FLinearColor(0.08f, 0.08f, 0.08f, 0.85f), X, Y, W, H);           // back
	DrawRect(Fill, X, Y, W * FMath::Clamp(Fraction, 0.f, 1.f), H);            // fill
	const FString Text = FString::Printf(TEXT("%s %.0f / %.0f"), Label, Current, Max);
	DrawText(Text, FLinearColor::White, X + 6, Y + H * 0.5f - 8.f, nullptr, 1.0f);
}

void AKeldranHUD::DrawHUD()
{
	Super::DrawHUD();
	if (!Canvas)
	{
		return;
	}

	const float SizeX = Canvas->SizeX;
	const float SizeY = Canvas->SizeY;

	// --- Resource bars, bottom-left ---
	if (UAbilitySystemComponent* ASC = GetPlayerASC())
	{
		auto Attr = [ASC](const FGameplayAttribute& A) { return ASC->GetNumericAttribute(A); };
		const float H  = Attr(UKeldranAttributeSet::GetHealthAttribute());
		const float HM = FMath::Max(1.f, Attr(UKeldranAttributeSet::GetMaxHealthAttribute()));
		const float M  = Attr(UKeldranAttributeSet::GetManaAttribute());
		const float MM = FMath::Max(1.f, Attr(UKeldranAttributeSet::GetMaxManaAttribute()));
		const float S  = Attr(UKeldranAttributeSet::GetStaminaAttribute());
		const float SM = FMath::Max(1.f, Attr(UKeldranAttributeSet::GetMaxStaminaAttribute()));

		const float BX = 40.f, BW = 300.f, BH = 24.f;
		float BY = SizeY - 3 * (BH + 10.f) - 40.f;
		DrawBar(BX, BY, BW, BH, H / HM, FLinearColor(0.82f, 0.10f, 0.10f, 0.95f), TEXT("HP"), H, HM);
		BY += BH + 10.f;
		DrawBar(BX, BY, BW, BH, M / MM, FLinearColor(0.15f, 0.35f, 0.92f, 0.95f), TEXT("MP"), M, MM);
		BY += BH + 10.f;
		DrawBar(BX, BY, BW, BH, S / SM, FLinearColor(0.95f, 0.75f, 0.10f, 0.95f), TEXT("SP"), S, SM);
	}

	// --- Action bar, bottom-center ---
	const float IconSize = 64.f, Pad = 8.f;
	const int32 Count = AbilityIcons.Num();
	if (Count > 0)
	{
		const float TotalW = Count * IconSize + (Count - 1) * Pad;
		float IX = (SizeX - TotalW) * 0.5f;
		const float IY = SizeY - IconSize - 28.f;
		for (int32 i = 0; i < Count; ++i)
		{
			DrawRect(FLinearColor(0.f, 0.f, 0.f, 0.55f), IX - 2, IY - 2, IconSize + 4, IconSize + 4);
			DrawTexture(AbilityIcons[i], IX, IY, IconSize, IconSize, 0.f, 0.f, 1.f, 1.f);
			DrawText(FString::Printf(TEXT("%d"), i + 1), FLinearColor::White, IX + 4, IY + 2);
			IX += IconSize + Pad;
		}
	}
}
