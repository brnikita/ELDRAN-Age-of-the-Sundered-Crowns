// Copyright KELDRAN.
#include "KeldranAssetManager.h"

UKeldranAssetManager& UKeldranAssetManager::Get()
{
	check(GEngine);
	UKeldranAssetManager* Singleton = Cast<UKeldranAssetManager>(GEngine->AssetManager);
	checkf(Singleton, TEXT("AssetManagerClassName must be set to KeldranAssetManager in DefaultEngine.ini"));
	return *Singleton;
}

void UKeldranAssetManager::StartInitialLoading()
{
	Super::StartInitialLoading();
	// Primary asset type scan rules live in DefaultGame.ini; data-integrity is enforced by the
	// Keldran.Data.Integrity automation test (Docs/07_TestPlan.md).
}
