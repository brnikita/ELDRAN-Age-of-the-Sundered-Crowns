// Copyright KELDRAN.
#include "KeldranGatewayClient.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

UKeldranGatewayClient::UKeldranGatewayClient()
{
	BaseUrl = TEXT("http://127.0.0.1:8080");
}

void UKeldranGatewayClient::SendJson(const FString& Verb, const FString& Path,
	const TSharedRef<FJsonObject>& Body, const FString& AuthBearer,
	TFunction<void(bool, int32, TSharedPtr<FJsonObject>)> OnComplete)
{
	FString Payload;
	const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Payload);
	FJsonSerializer::Serialize(Body, Writer);

	const TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(BaseUrl / Path);
	Request->SetVerb(Verb);
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	if (!AuthBearer.IsEmpty())
	{
		Request->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *AuthBearer));
	}
	Request->SetContentAsString(Payload);

	Request->OnProcessRequestComplete().BindLambda(
		[OnComplete](FHttpRequestPtr Req, FHttpResponsePtr Resp, bool bOk)
		{
			if (!bOk || !Resp.IsValid())
			{
				OnComplete(false, 0, nullptr);
				return;
			}
			TSharedPtr<FJsonObject> Json;
			const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Resp->GetContentAsString());
			FJsonSerializer::Deserialize(Reader, Json);
			OnComplete(true, Resp->GetResponseCode(), Json);
		});
	Request->ProcessRequest();
}

void UKeldranGatewayClient::Login(const FString& Email, const FString& Password,
	const FKeldranLoginDelegate& OnComplete)
{
	TSharedRef<FJsonObject> Body = MakeShared<FJsonObject>();
	Body->SetStringField(TEXT("email"), Email);
	Body->SetStringField(TEXT("password"), Password);

	FKeldranLoginDelegate Callback = OnComplete;
	SendJson(TEXT("POST"), TEXT("auth/login"), Body, FString(),
		[Callback](bool bNetOk, int32 Code, TSharedPtr<FJsonObject> Json)
		{
			FKeldranLoginResult Result;
			if (bNetOk && Code == 200 && Json.IsValid())
			{
				Result.bSuccess = true;
				Result.SessionToken = Json->GetStringField(TEXT("session_token"));
				Result.AccountId = Json->GetStringField(TEXT("account_id"));
			}
			else
			{
				Result.Error = bNetOk ? FString::Printf(TEXT("HTTP %d"), Code) : TEXT("network error");
			}
			Callback.ExecuteIfBound(Result);
		});
}

void UKeldranGatewayClient::RequestPlay(const FString& SessionToken, const FString& CharacterId,
	TFunction<void(const FKeldranPlayResult&)> OnComplete)
{
	TSharedRef<FJsonObject> Body = MakeShared<FJsonObject>();
	Body->SetStringField(TEXT("character_id"), CharacterId);

	SendJson(TEXT("POST"), TEXT("play"), Body, SessionToken,
		[OnComplete](bool bNetOk, int32 Code, TSharedPtr<FJsonObject> Json)
		{
			FKeldranPlayResult Result;
			if (bNetOk && Code == 200 && Json.IsValid())
			{
				Result.bSuccess = true;
				Result.ShardAddress = Json->GetStringField(TEXT("shard_address"));
				Result.Ticket = Json->GetStringField(TEXT("ticket"));
				Result.ZoneId = Json->GetStringField(TEXT("zone_id"));
			}
			else
			{
				Result.Error = bNetOk ? FString::Printf(TEXT("HTTP %d"), Code) : TEXT("network error");
			}
			OnComplete(Result);
		});
}

void UKeldranGatewayClient::VerifyTicket(const FString& Ticket,
	TFunction<void(bool, const FString&, const FString&)> OnComplete)
{
	TSharedRef<FJsonObject> Body = MakeShared<FJsonObject>();
	Body->SetStringField(TEXT("ticket"), Ticket);

	SendJson(TEXT("POST"), TEXT("auth/verify"), Body, FString(),
		[OnComplete](bool bNetOk, int32 Code, TSharedPtr<FJsonObject> Json)
		{
			if (bNetOk && Code == 200 && Json.IsValid())
			{
				OnComplete(true, Json->GetStringField(TEXT("account_id")),
					Json->GetStringField(TEXT("character_id")));
			}
			else
			{
				OnComplete(false, FString(), FString());
			}
		});
}

void UKeldranGatewayClient::LoadCharacter(const FString& CharacterId,
	TFunction<void(bool, TSharedPtr<FJsonObject>)> OnComplete)
{
	const TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(BaseUrl / TEXT("persistence") / CharacterId);
	Request->SetVerb(TEXT("GET"));
	Request->OnProcessRequestComplete().BindLambda(
		[OnComplete](FHttpRequestPtr Req, FHttpResponsePtr Resp, bool bOk)
		{
			if (bOk && Resp.IsValid() && Resp->GetResponseCode() == 200)
			{
				TSharedPtr<FJsonObject> Json;
				const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Resp->GetContentAsString());
				if (FJsonSerializer::Deserialize(Reader, Json) && Json.IsValid())
				{
					OnComplete(true, Json);
					return;
				}
			}
			OnComplete(false, nullptr);
		});
	Request->ProcessRequest();
}

void UKeldranGatewayClient::SaveCharacter(const TSharedRef<FJsonObject>& Payload,
	TFunction<void(bool)> OnComplete)
{
	SendJson(TEXT("POST"), TEXT("persistence/save"), Payload, FString(),
		[OnComplete](bool bNetOk, int32 Code, TSharedPtr<FJsonObject> Json)
		{
			OnComplete(bNetOk && Code == 200);
		});
}
