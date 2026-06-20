// Copyright KELDRAN. Mob AI (Docs/00 §6). Code-driven sight->chase->attack->leash so the slice
// runs without an editor-authored Behavior Tree (a StateTree asset can replace this later).
#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "KeldranAIController.generated.h"

class UAIPerceptionComponent;
class UAISenseConfig_Sight;

UCLASS()
class KELDRANAI_API AKeldranAIController : public AAIController
{
	GENERATED_BODY()

public:
	AKeldranAIController();

	virtual void OnPossess(APawn* InPawn) override;
	virtual void Tick(float DeltaSeconds) override;

protected:
	UPROPERTY(VisibleAnywhere, Category = "Keldran|AI")
	TObjectPtr<UAIPerceptionComponent> PerceptionComp;

	UPROPERTY()
	TObjectPtr<UAISenseConfig_Sight> SightConfig;

	UPROPERTY()
	TObjectPtr<AActor> CurrentTarget;

	FVector HomeLocation = FVector::ZeroVector;
	float AttackCooldown = 0.f;

	UFUNCTION()
	void OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

	void TryAttack();
};
