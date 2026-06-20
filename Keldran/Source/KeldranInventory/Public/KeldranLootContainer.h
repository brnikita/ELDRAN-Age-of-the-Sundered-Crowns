// Copyright KELDRAN. Server-owned loot drop (corpse/chest). Contents are rolled server-side
// (FKeldranLootService) and transferred to a looter's inventory on request (Docs/00 §4).
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "KeldranLootTypes.h"
#include "KeldranLootContainer.generated.h"

UCLASS()
class KELDRANINVENTORY_API AKeldranLootContainer : public AActor
{
	GENERATED_BODY()

public:
	AKeldranLootContainer();

	/** Server: set the rolled contents. */
	void InitContents(const FLootResult& InContents);

	/** Server: move all contents into the looter's inventory; destroy when empty. */
	void LootAll(AActor* Looter);

	UFUNCTION(BlueprintPure, Category = "Keldran|Loot")
	const FLootResult& GetContents() const { return Contents; }

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	UPROPERTY(ReplicatedUsing = OnRep_Contents)
	FLootResult Contents;

	UFUNCTION()
	void OnRep_Contents();
};
