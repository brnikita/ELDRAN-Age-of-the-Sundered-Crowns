// Copyright KELDRAN.
#include "KeldranNpcCharacter.h"
#include "KeldranDialogueComponent.h"

AKeldranNpcCharacter::AKeldranNpcCharacter()
{
	PrimaryActorTick.bCanEverTick = false;
	Dialogue = CreateDefaultSubobject<UKeldranDialogueComponent>(TEXT("Dialogue"));
}
