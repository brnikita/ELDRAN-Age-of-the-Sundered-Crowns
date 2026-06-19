// Copyright KELDRAN.
#include "KeldranGameInstance.h"

void UKeldranGameInstance::Init()
{
	Super::Init();
	UE_LOG(LogTemp, Log, TEXT("[Keldran] GameInstance initialized."));
}

void UKeldranGameInstance::Shutdown()
{
	UE_LOG(LogTemp, Log, TEXT("[Keldran] GameInstance shutdown."));
	Super::Shutdown();
}
