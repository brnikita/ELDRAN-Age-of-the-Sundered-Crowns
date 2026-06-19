// Copyright KELDRAN. Asset manager — registers primary asset types (Docs/00 §1).
#pragma once

#include "CoreMinimal.h"
#include "Engine/AssetManager.h"
#include "KeldranAssetManager.generated.h"

UCLASS()
class KELDRANCORE_API UKeldranAssetManager : public UAssetManager
{
	GENERATED_BODY()

public:
	static UKeldranAssetManager& Get();

	virtual void StartInitialLoading() override;
};
