// Copyright KELDRAN.
#include "KeldranGameplayAbility.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemGlobals.h"
#include "GameFramework/Pawn.h"
#include "Engine/OverlapResult.h"
#include "KeldranGameplayTags.h"

UKeldranGameplayAbility::UKeldranGameplayAbility()
{
	// Instanced per actor: standard for MMO abilities that hold per-activation state.
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	// Server activates; client predicts where supported.
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

AActor* UKeldranGameplayAbility::FindMeleeTargetActor(float Range) const
{
	const AActor* Avatar = GetAvatarActorFromActorInfo();
	if (!Avatar)
	{
		return nullptr;
	}
	const UWorld* World = Avatar->GetWorld();
	if (!World)
	{
		return nullptr;
	}

	const FVector Origin = Avatar->GetActorLocation();
	const FVector Forward = Avatar->GetActorForwardVector();
	const FVector Center = Origin + Forward * (Range * 0.5f);

	TArray<FOverlapResult> Overlaps;
	FCollisionShape Sphere = FCollisionShape::MakeSphere(Range);
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Avatar);
	World->OverlapMultiByObjectType(Overlaps, Center, FQuat::Identity,
		FCollisionObjectQueryParams(ECC_Pawn), Sphere, Params);

	AActor* Best = nullptr;
	float BestDistSq = TNumericLimits<float>::Max();
	for (const FOverlapResult& O : Overlaps)
	{
		AActor* Actor = O.GetActor();
		if (!Actor || Actor == Avatar)
		{
			continue;
		}
		if (!UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Actor))
		{
			continue;
		}
		const float DistSq = FVector::DistSquared(Origin, Actor->GetActorLocation());
		if (DistSq < BestDistSq)
		{
			BestDistSq = DistSq;
			Best = Actor;
		}
	}
	return Best;
}

void UKeldranGameplayAbility::ApplyEffectToActor(AActor* TargetActor,
	TSubclassOf<UGameplayEffect> EffectClass, float DamageMagnitude)
{
	if (!TargetActor || !EffectClass)
	{
		return;
	}
	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
	UAbilitySystemComponent* TargetASC =
		UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(TargetActor);
	if (!SourceASC || !TargetASC)
	{
		return;
	}

	FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
	Context.AddSourceObject(this);
	FGameplayEffectSpecHandle Spec = SourceASC->MakeOutgoingSpec(EffectClass, GetAbilityLevel(), Context);
	if (Spec.IsValid())
	{
		if (DamageMagnitude > 0.f)
		{
			Spec.Data->SetSetByCallerMagnitude(Tag_Data_Damage, DamageMagnitude);
		}
		SourceASC->ApplyGameplayEffectSpecToTarget(*Spec.Data.Get(), TargetASC);
	}
}
