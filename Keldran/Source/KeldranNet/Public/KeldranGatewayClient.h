// Copyright KELDRAN. HTTP client to the Login/Account Gateway (Docs/02_NetworkDesign.md §2).
// Client uses it for login/characters/play; server uses it for /auth/verify + persistence.
#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "KeldranGatewayClient.generated.h"

/** Result of a login attempt. */
USTRUCT(BlueprintType)
struct FKeldranLoginResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Keldran|Net")
	bool bSuccess = false;

	UPROPERTY(BlueprintReadOnly, Category = "Keldran|Net")
	FString SessionToken;

	UPROPERTY(BlueprintReadOnly, Category = "Keldran|Net")
	FString AccountId;

	UPROPERTY(BlueprintReadOnly, Category = "Keldran|Net")
	FString Error;
};

// Single-cast dynamic delegate: usable as a BlueprintCallable function parameter (callback).
DECLARE_DYNAMIC_DELEGATE_OneParam(FKeldranLoginDelegate, const FKeldranLoginResult&, Result);

/**
 * Thin async HTTP wrapper around the gateway. The base URL defaults to the local dev gateway
 * (http://127.0.0.1:8080) and is overridable. Never holds secrets; tokens flow through results.
 */
UCLASS(BlueprintType)
class KELDRANNET_API UKeldranGatewayClient : public UObject
{
	GENERATED_BODY()

public:
	UKeldranGatewayClient();

	/** Base URL of the gateway, e.g. "http://127.0.0.1:8080". */
	UPROPERTY(BlueprintReadWrite, Category = "Keldran|Net")
	FString BaseUrl;

	/** POST /auth/login {email,password}. Fires OnComplete with the parsed result. */
	UFUNCTION(BlueprintCallable, Category = "Keldran|Net")
	void Login(const FString& Email, const FString& Password, const FKeldranLoginDelegate& OnComplete);
};
