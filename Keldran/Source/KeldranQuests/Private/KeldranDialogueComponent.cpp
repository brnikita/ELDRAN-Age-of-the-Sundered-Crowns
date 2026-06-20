// Copyright KELDRAN.
#include "KeldranDialogueComponent.h"
#include "KeldranDialogueServiceClient.h"

UKeldranDialogueComponent::UKeldranDialogueComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

FText UKeldranDialogueComponent::FirstAuthoredOr(const TCHAR* Default) const
{
	return AuthoredLines.Num() > 0 ? AuthoredLines[0] : FText::FromString(Default);
}

void UKeldranDialogueComponent::Talk(const FString& AccountId, const FString& Utterance,
	const FKeldranLineDelegate& OnLine)
{
	const FText Fallback = FirstAuthoredOr(TEXT("..."));

	if (!bIsLivingNPC || NpcId.IsNone())
	{
		OnLine.ExecuteIfBound(Fallback);
		return;
	}

	if (!ServiceClient)
	{
		ServiceClient = NewObject<UKeldranDialogueServiceClient>(this);
	}

	FKeldranLineDelegate Callback = OnLine;
	const FString FallbackStr = Fallback.ToString();
	ServiceClient->RequestLine(NpcId.ToString(), AccountId, Utterance, FallbackStr,
		[Callback](const FString& Line, bool /*bFromService*/)
		{
			Callback.ExecuteIfBound(FText::FromString(Line));
		});
}
