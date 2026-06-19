// Copyright KELDRAN.
#include "KeldranSessionSubsystem.h"

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
		// M1-9: request /play -> shard address + ticket, then ClientTravel.
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[Keldran] Login failed: %s"), *Result.Error);
	}
}
