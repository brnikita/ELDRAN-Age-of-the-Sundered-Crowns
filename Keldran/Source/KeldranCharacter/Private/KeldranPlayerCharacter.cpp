// Copyright KELDRAN.
#include "KeldranPlayerCharacter.h"
#include "KeldranPlayerState.h"
#include "KeldranAttributeSet.h"
#include "KeldranGameplayAbility.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"

AKeldranPlayerCharacter::AKeldranPlayerCharacter()
{
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength = 450.f;
	SpringArm->bUsePawnControlRotation = true;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);

	bUseControllerRotationYaw = false;
	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->bOrientRotationToMovement = true;
	}
}

void AKeldranPlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	InitAbilityActorInfoFromPlayerState(); // server
}

void AKeldranPlayerCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	InitAbilityActorInfoFromPlayerState(); // client
}

void AKeldranPlayerCharacter::InitAbilityActorInfoFromPlayerState()
{
	AKeldranPlayerState* PS = GetPlayerState<AKeldranPlayerState>();
	if (!PS)
	{
		return;
	}
	UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent();
	if (!ASC)
	{
		return;
	}
	AbilitySystemComponent = ASC;
	ASC->InitAbilityActorInfo(PS, this); // owner = PlayerState, avatar = this pawn

	if (HasAuthority() && !bAbilitiesInitialized)
	{
		bAbilitiesInitialized = true;

		UKeldranAttributeSet* Set = NewObject<UKeldranAttributeSet>(PS);
		ASC->AddSpawnedAttribute(Set);
		AttributeSet = Set;

		for (const TSubclassOf<UKeldranGameplayAbility>& Ability : StartingAbilities)
		{
			if (Ability)
			{
				ASC->GiveAbility(FGameplayAbilitySpec(Ability, 1));
			}
		}
	}
}

void AKeldranPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			if (InputMapping)
			{
				Subsystem->AddMappingContext(InputMapping, 0);
			}
		}
	}

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (MoveAction)
		{
			EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AKeldranPlayerCharacter::Move);
		}
		if (LookAction)
		{
			EIC->BindAction(LookAction, ETriggerEvent::Triggered, this, &AKeldranPlayerCharacter::Look);
		}
	}
}

void AKeldranPlayerCharacter::Move(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();
	if (Controller)
	{
		const FRotator YawRot(0, Controller->GetControlRotation().Yaw, 0);
		const FVector Forward = FRotationMatrix(YawRot).GetUnitAxis(EAxis::X);
		const FVector Right = FRotationMatrix(YawRot).GetUnitAxis(EAxis::Y);
		AddMovementInput(Forward, Axis.Y);
		AddMovementInput(Right, Axis.X);
	}
}

void AKeldranPlayerCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();
	AddControllerYawInput(Axis.X);
	AddControllerPitchInput(Axis.Y);
}
