// Copyright KELDRAN.
#include "KeldranPlayerCharacter.h"
#include "KeldranPlayerState.h"
#include "KeldranAttributeSet.h"
#include "KeldranGameplayAbility.h"
#include "KeldranWardenAbilities.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "KeldranInventoryComponent.h"
#include "KeldranEquipmentComponent.h"
#include "KeldranQuestComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "KeldranAbilitySystemComponent.h"
#include "KeldranGameplayTags.h"

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

	Inventory = CreateDefaultSubobject<UKeldranInventoryComponent>(TEXT("Inventory"));
	Equipment = CreateDefaultSubobject<UKeldranEquipmentComponent>(TEXT("Equipment"));
	Quests = CreateDefaultSubobject<UKeldranQuestComponent>(TEXT("Quests"));

	// Slice Warden kit, granted on possession (designers can override in a BP child / data).
	StartingAbilities.Add(UGA_BasicAttack::StaticClass());
	StartingAbilities.Add(UGA_ShieldBash::StaticClass());
	StartingAbilities.Add(UGA_DefensiveStance::StaticClass());

	// Default Enhanced Input assets (generated under /Game/Input); overridable in data/BP.
	InputMapping = TSoftObjectPtr<UInputMappingContext>(FSoftObjectPath(TEXT("/Game/Input/IMC_Warden.IMC_Warden")));
	MoveAction   = TSoftObjectPtr<UInputAction>(FSoftObjectPath(TEXT("/Game/Input/IA_Move.IA_Move")));
	LookAction   = TSoftObjectPtr<UInputAction>(FSoftObjectPath(TEXT("/Game/Input/IA_Look.IA_Look")));
	AbilityActions.Add(TSoftObjectPtr<UInputAction>(FSoftObjectPath(TEXT("/Game/Input/IA_Ability1.IA_Ability1"))));
	AbilityActions.Add(TSoftObjectPtr<UInputAction>(FSoftObjectPath(TEXT("/Game/Input/IA_Ability2.IA_Ability2"))));
	AbilityActions.Add(TSoftObjectPtr<UInputAction>(FSoftObjectPath(TEXT("/Game/Input/IA_Ability3.IA_Ability3"))));
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

		int32 Granted = 0;
		for (const TSubclassOf<UKeldranGameplayAbility>& Ability : StartingAbilities)
		{
			if (Ability)
			{
				ASC->GiveAbility(FGameplayAbilitySpec(Ability, 1));
				++Granted;
			}
		}
		UE_LOG(LogTemp, Warning, TEXT("[Keldran] Player pawn possessed + ASC init on server (abilities granted: %d)"), Granted);
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
			if (UInputMappingContext* IMC = InputMapping.LoadSynchronous())
			{
				Subsystem->AddMappingContext(IMC, 0);
			}
		}
	}

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (UInputAction* Move_IA = MoveAction.LoadSynchronous())
		{
			EIC->BindAction(Move_IA, ETriggerEvent::Triggered, this, &AKeldranPlayerCharacter::Move);
		}
		if (UInputAction* Look_IA = LookAction.LoadSynchronous())
		{
			EIC->BindAction(Look_IA, ETriggerEvent::Triggered, this, &AKeldranPlayerCharacter::Look);
		}
		void (AKeldranPlayerCharacter::* Handlers[3])() = {
			&AKeldranPlayerCharacter::OnAbility1,
			&AKeldranPlayerCharacter::OnAbility2,
			&AKeldranPlayerCharacter::OnAbility3 };
		for (int32 i = 0; i < AbilityActions.Num() && i < 3; ++i)
		{
			if (UInputAction* Ability_IA = AbilityActions[i].LoadSynchronous())
			{
				EIC->BindAction(Ability_IA, ETriggerEvent::Started, this, Handlers[i]);
			}
		}
	}
}

void AKeldranPlayerCharacter::ActivateAbilitySlot(int32 SlotIndex)
{
	static const FGameplayTag SlotTags[3] = {
		Tag_Input_Ability_Slot1, Tag_Input_Ability_Slot2, Tag_Input_Ability_Slot3 };
	if (SlotIndex < 0 || SlotIndex >= 3)
	{
		return;
	}
	if (UKeldranAbilitySystemComponent* KASC = Cast<UKeldranAbilitySystemComponent>(AbilitySystemComponent))
	{
		KASC->ActivateAbilityByInputTag(SlotTags[SlotIndex]);
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
