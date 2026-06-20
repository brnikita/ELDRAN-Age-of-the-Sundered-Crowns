// Copyright KELDRAN.
#include "KeldranAIController.h"
#include "KeldranMobCharacter.h"
#include "KeldranWardenAbilities.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Character.h"

AKeldranAIController::AKeldranAIController()
{
	PrimaryActorTick.bCanEverTick = true;

	PerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("Perception"));
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("Sight"));
	SightConfig->SightRadius = 1200.f;
	SightConfig->LoseSightRadius = 1600.f;
	SightConfig->PeripheralVisionAngleDegrees = 90.f;
	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
	PerceptionComp->ConfigureSense(*SightConfig);
	PerceptionComp->SetDominantSense(SightConfig->GetSenseImplementation());
	SetPerceptionComponent(*PerceptionComp);
}

void AKeldranAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	if (InPawn)
	{
		HomeLocation = InPawn->GetActorLocation();
	}
	PerceptionComp->OnTargetPerceptionUpdated.AddDynamic(this, &AKeldranAIController::OnPerceptionUpdated);
}

void AKeldranAIController::OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	// Only chase player-controlled pawns; ignore other mobs.
	const APawn* AsPawn = Cast<APawn>(Actor);
	if (Stimulus.WasSuccessfullySensed() && AsPawn && AsPawn->IsPlayerControlled())
	{
		CurrentTarget = Actor;
	}
}

void AKeldranAIController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	AttackCooldown = FMath::Max(0.f, AttackCooldown - DeltaSeconds);

	AKeldranMobCharacter* Mob = Cast<AKeldranMobCharacter>(GetPawn());
	if (!Mob || Mob->IsDead())
	{
		return;
	}

	// Leash: return home if dragged too far.
	if (FVector::Dist(Mob->GetActorLocation(), HomeLocation) > 2000.f)
	{
		CurrentTarget = nullptr;
		MoveToLocation(HomeLocation);
		return;
	}

	if (!CurrentTarget)
	{
		return;
	}

	const float Dist = FVector::Dist(Mob->GetActorLocation(), CurrentTarget->GetActorLocation());
	if (Dist <= 180.f)
	{
		StopMovement();
		SetFocus(CurrentTarget);
		TryAttack();
	}
	else
	{
		MoveToActor(CurrentTarget, 150.f);
	}
}

void AKeldranAIController::TryAttack()
{
	if (AttackCooldown > 0.f)
	{
		return;
	}
	if (AKeldranMobCharacter* Mob = Cast<AKeldranMobCharacter>(GetPawn()))
	{
		if (UAbilitySystemComponent* ASC = Mob->GetAbilitySystemComponent())
		{
			ASC->TryActivateAbilityByClass(UGA_BasicAttack::StaticClass());
			AttackCooldown = 1.5f;
		}
	}
}
