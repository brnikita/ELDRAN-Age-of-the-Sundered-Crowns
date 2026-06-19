// Copyright KELDRAN. Process-lifetime game instance (Docs/00_ImplementationSpec.md §1).
#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "KeldranGameInstance.generated.h"

UCLASS()
class KELDRANCORE_API UKeldranGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void Init() override;
	virtual void Shutdown() override;
};
