// Copyright KELDRAN. Client-side login/session orchestration (Docs/02 §2):
// login -> gateway -> {token, shard} -> ClientTravel to the dedicated server.
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "KeldranGatewayClient.h"
#include "KeldranSessionSubsystem.generated.h"

UCLASS()
class KELDRANNET_API UKeldranSessionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** Begin the login flow against the gateway. */
	UFUNCTION(BlueprintCallable, Category = "Keldran|Net")
	void BeginLogin(const FString& Email, const FString& Password);

	UPROPERTY(BlueprintReadOnly, Category = "Keldran|Net")
	FString CachedSessionToken;

	UPROPERTY(BlueprintReadOnly, Category = "Keldran|Net")
	FString CachedAccountId;

private:
	UPROPERTY()
	TObjectPtr<UKeldranGatewayClient> Gateway;

	UFUNCTION()
	void HandleLoginComplete(const FKeldranLoginResult& Result);
};
