// Copyright KELDRAN. Reach-objective trigger (Docs/00 §5). Placed in the level; on player overlap
// it credits the matching Reach objective via the player's quest component.
#pragma once

#include "CoreMinimal.h"
#include "Engine/TriggerVolume.h"
#include "KeldranAreaMarker.generated.h"

UCLASS()
class KELDRANQUESTS_API AKeldranAreaMarker : public ATriggerVolume
{
	GENERATED_BODY()

public:
	AKeldranAreaMarker();

	/** Marker id matched against Reach objectives' TargetId (e.g. "marker.hollows-toe"). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Keldran|Quests")
	FName MarkerId;

	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;
};
