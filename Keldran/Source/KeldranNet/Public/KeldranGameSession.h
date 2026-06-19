// Copyright KELDRAN. Server-side login gate (Docs/02 §2 step 5). Validates the play ticket
// passed as a URL option before a client is admitted to the shard.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameSession.h"
#include "KeldranGameSession.generated.h"

UCLASS()
class KELDRANNET_API AKeldranGameSession : public AGameSession
{
	GENERATED_BODY()

public:
	/** Reject logins lacking a ticket (returns non-empty = rejection reason). Full async
	 *  verification with the gateway lands in M1-9. ApproveLogin is the AGameSession hook. */
	virtual FString ApproveLogin(const FString& Options) override;
};
