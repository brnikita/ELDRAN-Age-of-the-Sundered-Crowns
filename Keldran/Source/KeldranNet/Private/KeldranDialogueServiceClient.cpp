// Copyright KELDRAN.
#include "KeldranDialogueServiceClient.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

UKeldranDialogueServiceClient::UKeldranDialogueServiceClient()
{
	BaseUrl = TEXT("http://127.0.0.1:8090");
}

void UKeldranDialogueServiceClient::RequestLine(const FString& NpcId, const FString& AccountId,
	const FString& Utterance, const FString& FallbackLine,
	TFunction<void(const FString&, bool)> OnLine)
{
	TSharedRef<FJsonObject> Body = MakeShared<FJsonObject>();
	Body->SetStringField(TEXT("npc_id"), NpcId);
	Body->SetStringField(TEXT("account_id"), AccountId);
	Body->SetStringField(TEXT("player_utterance"), Utterance);
	FString Payload;
	const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Payload);
	FJsonSerializer::Serialize(Body, Writer);

	const TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(BaseUrl / TEXT("dialogue"));
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->SetContentAsString(Payload);
	Request->SetTimeout(9.0f);

	const FString Fallback = FallbackLine;
	Request->OnProcessRequestComplete().BindLambda(
		[OnLine, Fallback](FHttpRequestPtr Req, FHttpResponsePtr Resp, bool bOk)
		{
			if (bOk && Resp.IsValid() && Resp->GetResponseCode() == 200)
			{
				TSharedPtr<FJsonObject> Json;
				const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Resp->GetContentAsString());
				if (FJsonSerializer::Deserialize(Reader, Json) && Json.IsValid())
				{
					const FString Line = Json->GetStringField(TEXT("line"));
					if (!Line.IsEmpty())
					{
						OnLine(Line, true);
						return;
					}
				}
			}
			// Network/parse failure -> authored fallback (the service also falls back internally).
			OnLine(Fallback, false);
		});
	Request->ProcessRequest();
}
