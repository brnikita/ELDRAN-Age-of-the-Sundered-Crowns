// Copyright KELDRAN.
#include "KeldranGameSession.h"
#include "Kismet/GameplayStatics.h"

FString AKeldranGameSession::ApproveLogin(const FString& Options)
{
	const FString Ticket = UGameplayStatics::ParseOption(Options, TEXT("token"));
	if (Ticket.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("[Keldran] ApproveLogin rejected: no ticket."));
		return TEXT("Missing session ticket.");
	}
	// M1-9: POST /auth/verify {ticket} to the gateway and reject on failure. Skeleton accepts a
	// present ticket so the connection path can be exercised by tests.
	UE_LOG(LogTemp, Log, TEXT("[Keldran] ApproveLogin: ticket present, admitting (verify stub)."));
	return Super::ApproveLogin(Options);
}
