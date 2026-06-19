// Copyright KELDRAN.
#include "KeldranSessionSubsystem.h"
#include "Engine/GameInstance.h"
#include "GameFramework/PlayerController.h"

void UKeldranSessionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	Gateway = NewObject<UKeldranGatewayClient>(this);
}

void UKeldranSessionSubsystem::Deinitialize()
{
	Gateway = nullptr;
	Super::Deinitialize();
}

void UKeldranSessionSubsystem::BeginLogin(const FString& Email, const FString& Password)
{
	if (!Gateway)
	{
		return;
	}
	FKeldranLoginDelegate Done;
	Done.BindDynamic(this, &UKeldranSessionSubsystem::HandleLoginComplete);
	Gateway->Login(Email, Password, Done);
}

void UKeldranSessionSubsystem::HandleLoginComplete(const FKeldranLoginResult& Result)
{
	if (Result.bSuccess)
	{
		CachedSessionToken = Result.SessionToken;
		CachedAccountId = Result.AccountId;
		UE_LOG(LogTemp, Log, TEXT("[Keldran] Login OK for account %s"), *CachedAccountId);
		// Character selection (M2-16) then calls TravelToCharacter(CharacterId).
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[Keldran] Login failed: %s"), *Result.Error);
	}
}

void UKeldranSessionSubsystem::TravelToCharacter(const FString& CharacterId)
{
	if (!Gateway || CachedSessionToken.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("[Keldran] TravelToCharacter: not logged in."));
		return;
	}

	TWeakObjectPtr<UKeldranSessionSubsystem> WeakThis(this);
	Gateway->RequestPlay(CachedSessionToken, CharacterId,
		[WeakThis, CharacterId](const FKeldranPlayResult& Play)
		{
			if (!WeakThis.IsValid())
			{
				return;
			}
			if (!Play.bSuccess)
			{
				UE_LOG(LogTemp, Warning, TEXT("[Keldran] /play failed: %s"), *Play.Error);
				return;
			}
			UGameInstance* GI = WeakThis->GetGameInstance();
			APlayerController* PC = GI ? GI->GetFirstLocalPlayerController() : nullptr;
			if (!PC)
			{
				UE_LOG(LogTemp, Warning, TEXT("[Keldran] /play OK but no local PlayerController to travel."));
				return;
			}
			const FString Url = FString::Printf(TEXT("%s?token=%s&char=%s"),
				*Play.ShardAddress, *Play.Ticket, *CharacterId);
			UE_LOG(LogTemp, Log, TEXT("[Keldran] Traveling to shard: %s (zone %s)"),
				*Play.ShardAddress, *Play.ZoneId);
			PC->ClientTravel(Url, ETravelType::TRAVEL_Absolute);
		});
}
