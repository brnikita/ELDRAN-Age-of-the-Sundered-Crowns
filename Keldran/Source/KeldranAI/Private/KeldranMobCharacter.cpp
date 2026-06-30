// Copyright KELDRAN.
#include "KeldranMobCharacter.h"
#include "KeldranMobTypes.h"
#include "KeldranAbilitySystemComponent.h"
#include "KeldranAttributeSet.h"
#include "KeldranLootContainer.h"
#include "KeldranLootService.h"
#include "KeldranLootTypes.h"
#include "Engine/DataTable.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbility.h"
#include "KeldranGameplayTags.h"

AKeldranMobCharacter::AKeldranMobCharacter()
{
	UKeldranAbilitySystemComponent* ASC = CreateDefaultSubobject<UKeldranAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	ASC->SetIsReplicated(true);
	ASC->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);
	AbilitySystemComponent = ASC;
	AttributeSet = CreateDefaultSubobject<UKeldranAttributeSet>(TEXT("AttributeSet"));
}

void AKeldranMobCharacter::BeginPlay()
{
	Super::BeginPlay();
	if (!AbilitySystemComponent)
	{
		return;
	}
	AbilitySystemComponent->InitAbilityActorInfo(this, this);

	// Server (dedicated or standalone) initializes stats + death binding; not on net clients.
	if (GetNetMode() != NM_Client)
	{
		InitFromRow();
		// Reliable death signal: the AttributeSet broadcasts OnOutOfHealth at 0 HP.
		if (UKeldranAttributeSet* Set = Cast<UKeldranAttributeSet>(AttributeSet))
		{
			Set->OnOutOfHealth.AddUObject(this, &AKeldranMobCharacter::HandleDeath);
		}
	}
}

void AKeldranMobCharacter::InitFromRow()
{
	if (!MobTable || MobRow.IsNone())
	{
		return;
	}
	const FMobRow* Row = MobTable->FindRow<FMobRow>(MobRow, TEXT("Mob"), false);
	if (!Row)
	{
		return;
	}

	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->MaxWalkSpeed = Row->MovementSpeed;
	}
	if (UKeldranAttributeSet* Set = Cast<UKeldranAttributeSet>(AttributeSet))
	{
		Set->InitMaxHealth(Row->Health);
		Set->InitHealth(Row->Health);
		Set->InitAttackPower(Row->AttackPower);
		Set->InitArmor(Row->Armor);
	}
	for (const TSubclassOf<UGameplayAbility>& Ability : Row->Abilities)
	{
		if (Ability)
		{
			AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(Ability, Row->Level));
		}
	}
	if (USkeletalMesh* LoadedMesh = Row->SkeletalMesh.LoadSynchronous())
	{
		GetMesh()->SetSkeletalMesh(LoadedMesh);
	}

	CachedLootTable = Row->LootTable;
	CachedXPReward = Row->XPReward;
}

void AKeldranMobCharacter::OnDeadTagChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
	if (NewCount > 0)
	{
		HandleDeath();
	}
}

void AKeldranMobCharacter::HandleDeath()
{
	if (bDeathHandled || GetNetMode() == NM_Client)
	{
		return;
	}
	bDeathHandled = true;

	if (LootTableAsset && !CachedLootTable.IsNone())
	{
		const FLootTableRow* LT = LootTableAsset->FindRow<FLootTableRow>(CachedLootTable, TEXT("Death"), false);
		if (LT)
		{
			FRandomStream Rng(FMath::Rand());
			const FLootResult Loot = FKeldranLootService::RollLoot(*LT, Rng, /*bIncludeQuestDrops=*/true);
			if (UWorld* World = GetWorld())
			{
				AKeldranLootContainer* Container = World->SpawnActor<AKeldranLootContainer>(
					GetActorLocation(), GetActorRotation());
				if (Container)
				{
					Container->InitContents(Loot);
				}
			}
		}
	}

	// Quest kill credit + XP are awarded by the killer's systems (wired in M2 functional pass).
	SetLifeSpan(30.f);
}
