// Copyright KELDRAN.
#include "KeldranGatewayClient.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

UKeldranGatewayClient::UKeldranGatewayClient()
{
	BaseUrl = TEXT("http://127.0.0.1:8080");
}

void UKeldranGatewayClient::Login(const FString& Email, const FString& Password, const FKeldranLoginDelegate& OnComplete)
{
	TSharedRef<FJsonObject> Body = MakeShared<FJsonObject>();
	Body->SetStringField(TEXT("email"), Email);
	Body->SetStringField(TEXT("password"), Password);
	FString Payload;
	const TSharedRef<TJsonWriter<>> W = TJsonWriterFactory<>::Create(&Payload);
	FJsonSerializer::Serialize(Body, W);

	const TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(BaseUrl / TEXT("auth/login"));
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->SetContentAsString(Payload);

	FKeldranLoginDelegate Callback = OnComplete;
	Request->OnProcessRequestComplete().BindLambda(
		[Callback](FHttpRequestPtr Req, FHttpResponsePtr Resp, bool bOk)
		{
			FKeldranLoginResult Result;
			if (!bOk || !Resp.IsValid())
			{
				Result.Error = TEXT("network error");
				Callback.ExecuteIfBound(Result);
				return;
			}
			const int32 Code = Resp->GetResponseCode();
			TSharedPtr<FJsonObject> Json;
			const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Resp->GetContentAsString());
			FJsonSerializer::Deserialize(Reader, Json);
			if (Code == 200 && Json.IsValid())
			{
				Result.bSuccess = true;
				Result.SessionToken = Json->GetStringField(TEXT("session_token"));
				Result.AccountId = Json->GetStringField(TEXT("account_id"));
			}
			else
			{
				Result.Error = FString::Printf(TEXT("HTTP %d"), Code);
			}
			Callback.ExecuteIfBound(Result);
		});
	Request->ProcessRequest();
}
