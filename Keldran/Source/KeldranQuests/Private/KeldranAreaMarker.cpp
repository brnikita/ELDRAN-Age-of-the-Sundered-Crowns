// Copyright KELDRAN.
#include "KeldranAreaMarker.h"
#include "KeldranQuestComponent.h"

AKeldranAreaMarker::AKeldranAreaMarker()
{
	bReplicates = false; // server evaluates overlaps; effect is on the server-side quest component
}

void AKeldranAreaMarker::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);
	if (!HasAuthority() || !OtherActor || MarkerId.IsNone())
	{
		return;
	}
	if (UKeldranQuestComponent* Quests = OtherActor->FindComponentByClass<UKeldranQuestComponent>())
	{
		Quests->NotifyReach(MarkerId);
	}
}
