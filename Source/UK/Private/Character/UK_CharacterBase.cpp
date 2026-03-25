// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/UK_CharacterBase.h"
#include "Character/UK_PlayerController.h"
#include "Character/UK_PlayerState.h"
#include "ActorComponent/CustomCharacterMovementComponent.h"
#include "Character/Weapon/UK_WeaponBase.h"
#include "AIMonster/AIMonsterBase.h"
#include "InputAction.h"
#include "Tags/UK_GameplayTags.h"
#include "ActorComponent/UK_InventoryComponent.h"
#include "NPC/Component/UK_InteractionComponent.h"
#include "NPC/Component/UK_QuestComponent.h"
#include "DataAsset/UK_WeaponData.h"
#include "DataAsset/UK_StatusAnimData.h"
#include "DataAsset/UK_InputConfig.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "AbilitySystemComponent.h"
#include "FrameTypes.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/OverlapResult.h"
#include "Sound/SoundAttenuation.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Character/AttibuteSet/UK_PlayerStatusAttributeSet.h"
#include "Components/CapsuleComponent.h"
#include "DataAsset/Data/UK_WeaponItemData.h"
#include "Kismet/GameplayStatics.h"
#include "Systems/Data/UK_InGameSave.h"
#include "Systems/Sound/UK_SoundManager.h"
#include "ProfilingDebugging/CpuProfilerTrace.h"

#pragma region Defualt
DECLARE_CYCLE_STAT(TEXT("UK Character Logic"), STAT_UKCharacter, STATGROUP_UK_Character);
DECLARE_CYCLE_STAT(TEXT("Constructor"), Constructor, STATGROUP_UK_Character);

// 무현님 대머리 ㅋㅋ
// Sets default values
AUK_CharacterBase::AUK_CharacterBase(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer.SetDefaultSubobjectClass<UCustomCharacterMovementComponent>(
		ACharacter::CharacterMovementComponentName)),
	GlideFallSpeed(200.f),
	SprintSpeed(800.f),
	bIsLock(false),
	NowWeapon(nullptr),
	WeaponSlotIndex(0)
{
	SCOPE_CYCLE_COUNTER(Constructor);
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = false;
	bIsClimb = false;
	GetMesh()->SetRelativeLocationAndRotation(
		FVector(0.f, 0.f, -90.f),
		FRotator(0.f, -90.f, 0.f));
	GetMesh()->SetCollisionProfileName(TEXT("UK_Charactor"));
	GetMesh()->VisibilityBasedAnimTickOption =
		EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
#pragma region SpringArm

	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArmComp->SetupAttachment(GetRootComponent());
	SpringArmComp->TargetArmLength = 450.f;
	SpringArmComp->SetRelativeLocation(FVector(0.f, 20.f, 60.f));
	SpringArmComp->bUsePawnControlRotation = true;
	SpringArmComp->bEnableCameraLag = true; // 카메라가 캐릭터를 뒤늦게 따라옴
	SpringArmComp->CameraLagSpeed = 5.0f; // 따라오는 속도
	SpringArmComp->CameraLagMaxDistance = 80.0f; // 카메라와 본래 위치와의 최대 거리 차이

#pragma endregion

#pragma region Camera

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArmComp);
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 480.f, 0.0f);

#pragma endregion

	SkeletalMeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMeshComp"));
	SkeletalMeshComp->SetupAttachment(GetMesh());

	RightHandWeaponComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("RightHandWeaponComponent"));
	RightHandWeaponComponent->SetupAttachment(SkeletalMeshComp, TEXT("Weapon_rSocket"));
	RightHandWeaponComponent->SetLeaderPoseComponent(SkeletalMeshComp);

	LeftHandWeaponComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("LeftHandWeaponComponent"));
	LeftHandWeaponComponent->SetupAttachment(SkeletalMeshComp, TEXT("Weapon_lSocket"));
	LeftHandWeaponComponent->SetLeaderPoseComponent(SkeletalMeshComp);

	InventoryComponent = CreateDefaultSubobject<UUK_InventoryComponent>(TEXT("InventoryComponent"));
	InteractionComp = CreateDefaultSubobject<UUK_InteractionComponent>(TEXT("InteractionComponent"));
	QuestComp = CreateDefaultSubobject<UUK_QuestComponent>(TEXT("QuestComponent"));
}

// Called every frame
DECLARE_CYCLE_STAT(TEXT("Tick"), Tick, STATGROUP_UK_Character);

void AUK_CharacterBase::Tick(float DeltaTime)
{
	SCOPE_CYCLE_COUNTER(Tick);
	Super::Tick(DeltaTime);
	//UpdateMovementState();
}

DECLARE_CYCLE_STAT(TEXT("PossessedBy"), PossessedBy, STATGROUP_UK_Character);

void AUK_CharacterBase::PossessedBy(AController* NewController)
{
	SCOPE_CYCLE_COUNTER(PossessedBy);
	Super::PossessedBy(NewController);
}

DECLARE_CYCLE_STAT(TEXT("Landed"), Landed, STATGROUP_UK_Character);

void AUK_CharacterBase::Landed(const FHitResult& Hit)
{
	SCOPE_CYCLE_COUNTER(Landed);
	Super::Landed(Hit);
	FGameplayEventData EventData;
	EventData.EventTag = UK_GameplayTags::Action::DropAttack;
	GetAbilitySystemComponent()->HandleGameplayEvent(EventData.EventTag, &EventData);
}

DECLARE_CYCLE_STAT(TEXT("ChangedAttribute"), ChangedAttribute, STATGROUP_UK_Character);

void AUK_CharacterBase::ChangedAttribute(ECharacterAttribute NewAttribute)
{
	SCOPE_CYCLE_COUNTER(ChangedAttribute);
	Attribute = NewAttribute;
	OnChangedAttribute.Broadcast(NewAttribute);
}

DECLARE_CYCLE_STAT(TEXT("UpdateMovementState"), UpdateMovementState, STATGROUP_UK_Character);

void AUK_CharacterBase::UpdateMovementState()
{
	SCOPE_CYCLE_COUNTER(UpdateMovementState);
	ActorTrace();
	// UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	//
	// if (bInWater)
	// {
	// 	if (MoveComp->MovementMode != MOVE_Swimming)
	// 	{
	// 		MoveComp->SetMovementMode(MOVE_Swimming);
	// 	}
	// 	return;
	// }
	//
	// if (bWallDetected)
	// {
	// 	if (MoveComp->MovementMode != MOVE_Custom && MoveComp->CustomMovementMode != (uint8)
	// 		ECustomMovementMode::CMOVE_Climb)
	// 	{
	// 		MoveComp->SetMovementMode(MOVE_Custom, (uint8)ECustomMovementMode::CMOVE_Climb);
	// 	}
	// 	return;
	// }
	//
	// if (MoveComp->IsMovingOnGround() == false)
	// {
	// 	if (MoveComp->MovementMode != MOVE_Falling)
	// 	{
	// 		MoveComp->SetMovementMode(MOVE_Falling);
	// 	}
	// 	return;
	// }
}


DECLARE_CYCLE_STAT(TEXT("BeginPlay"), BeginPlay, STATGROUP_UK_Character);

void AUK_CharacterBase::BeginPlay()
{
	SCOPE_CYCLE_COUNTER(BeginPlay);

	Super::BeginPlay();

	InventoryComponent->OnChangedWeapon.AddDynamic(this, &ThisClass::SwapWeapon);
	PC = Cast<AUK_PlayerController>(GetController());
	DefualtGravity = GetCharacterMovement()->GravityScale;
	DefualtAirControl = GetCharacterMovement()->AirControl;
	if (IsValid(GetAbilitySystemComponent()))
	{
		GetAbilitySystemComponent()->InitAbilityActorInfo(GetPlayerState(), this);
		GiveStartupAbilities();
		AUK_PlayerState* PS = Cast<AUK_PlayerState>(GetPlayerState());
		if (IsValid(PS))
		{
			PS->InitializeAttributes();
		}
	}
	GetWorldTimerManager().SetTimer(
		DetectTimer,
		this,
		&AUK_CharacterBase::UpdateMonsterDetection,
		0.3f,
		true
	);

	GetWorldTimerManager().SetTimer(
		StaminaHealTimerHandle,
		this,
		&AUK_CharacterBase::HealStamina,
		0.1f,
		true
	);
	// GetWorldTimerManager().SetTimer(
	// 	WallTraceTimerHandler,
	// 	this,
	// 	&AUK_CharacterBase::UpdateMovementState,
	// 	0.5f,
	// 	true
	// );

	if (AUK_SoundManager::Get(GetWorld()))
	{
		SoundManager = AUK_SoundManager::Get(GetWorld());
	}
	else
	{
	}
}

DECLARE_CYCLE_STAT(TEXT("SetupPlayerInputComponent"), SetupPlayerInputComponent, STATGROUP_UK_Character);

void AUK_CharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	SCOPE_CYCLE_COUNTER(SetupPlayerInputComponent);

	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* UKInputComp = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (IsValid(UKInputComp) == false)
	{
		return;
	}
	if (IsValid(InputMappingConfig) == false)
		return;
	UKInputComp->BindAction(InputMappingConfig->FindNativeInputActionByTag(UK_GameplayTags::Input::Move),
	                        ETriggerEvent::Triggered, this, &ThisClass::Move);
	UKInputComp->BindAction(InputMappingConfig->FindNativeInputActionByTag(UK_GameplayTags::Input::Look),
	                        ETriggerEvent::Triggered, this, &AUK_CharacterBase::Look);
	UKInputComp->BindAction(InputMappingConfig->FindNativeInputActionByTag(UK_GameplayTags::Input::Jump),
	                        ETriggerEvent::Started, this, &ThisClass::Jump);
	UKInputComp->BindAction(InputMappingConfig->FindNativeInputActionByTag(UK_GameplayTags::Input::Jump),
	                        ETriggerEvent::Canceled, this, &ThisClass::StopJumping);
	UKInputComp->BindAction(InputMappingConfig->FindNativeInputActionByTag(UK_GameplayTags::Input::ZoomIn),
	                        ETriggerEvent::Triggered, this, &ThisClass::ZoomIn);
	UKInputComp->BindAction(InputMappingConfig->FindNativeInputActionByTag(UK_GameplayTags::Input::ZoomOut),
	                        ETriggerEvent::Triggered, this, &ThisClass::ZoomOut);
	UKInputComp->BindAction(InputMappingConfig->FindNativeInputActionByTag(UK_GameplayTags::Action::LightAttack),
	                        ETriggerEvent::Started, this, &ThisClass::LightAttack);
	UKInputComp->BindAction(InputMappingConfig->FindNativeInputActionByTag(UK_GameplayTags::Action::HeavyAttack),
	                        ETriggerEvent::Started, this, &ThisClass::HeavyAttack);
	UKInputComp->BindAction(InputMappingConfig->FindNativeInputActionByTag(UK_GameplayTags::Input::ToggleMouse),
	                        ETriggerEvent::Started, this, &ThisClass::ToggleMouse);
	UKInputComp->BindAction(InputMappingConfig->FindNativeInputActionByTag(UK_GameplayTags::Input::Interaction),
	                        ETriggerEvent::Started, this, &ThisClass::Interaction);
	UKInputComp->BindAction(InputMappingConfig->FindNativeInputActionByTag(UK_GameplayTags::Input::Setting),
	                        ETriggerEvent::Started, this, &ThisClass::Setting);
	UKInputComp->BindAction(InputMappingConfig->FindNativeInputActionByTag(UK_GameplayTags::Action::Swap1),
	                        ETriggerEvent::Started, this, &ThisClass::SlotWeaponOne);
	UKInputComp->BindAction(InputMappingConfig->FindNativeInputActionByTag(UK_GameplayTags::Action::Swap2),
	                        ETriggerEvent::Started, this, &ThisClass::SlotWeaponTwo);
	UKInputComp->BindAction(InputMappingConfig->FindNativeInputActionByTag(UK_GameplayTags::Action::Swap3),
	                        ETriggerEvent::Started, this, &ThisClass::SlotWeaponThree);
	UKInputComp->BindAction(InputMappingConfig->FindNativeInputActionByTag(UK_GameplayTags::Input::LockOnToggle),
	                        ETriggerEvent::Started, this, &ThisClass::LockONToggle);
	UKInputComp->BindAction(InputMappingConfig->FindNativeInputActionByTag(UK_GameplayTags::Input::NomalSkill),
	                        ETriggerEvent::Started, this, &ThisClass::NormalSkill);
	UKInputComp->BindAction(InputMappingConfig->FindNativeInputActionByTag(UK_GameplayTags::Input::UltimateSkill),
	                        ETriggerEvent::Started, this, &ThisClass::UltimateSkill);
	UKInputComp->BindAction(InputMappingConfig->FindNativeInputActionByTag(UK_GameplayTags::Action::Parry),
	                        ETriggerEvent::Started, this, &ThisClass::Parry);
	UKInputComp->BindAction(InputMappingConfig->FindNativeInputActionByTag(UK_GameplayTags::Input::Inventory),
	                        ETriggerEvent::Started, this, &ThisClass::Inventory);
	UKInputComp->BindAction(InputMappingConfig->FindNativeInputActionByTag(UK_GameplayTags::Input::WeaponCrafting),
	                        ETriggerEvent::Started, this, &ThisClass::WeaponCrafting);
	UKInputComp->BindAction(InputMappingConfig->FindNativeInputActionByTag(UK_GameplayTags::Input::Esc),
	                        ETriggerEvent::Started, this, &ThisClass::Esc);
}

// Called when the game starts or when spawned
DECLARE_CYCLE_STAT(TEXT("GetFloorDistance"), GetFloorDistance, STATGROUP_UK_Character);

float AUK_CharacterBase::GetFloorDistance()
{
	SCOPE_CYCLE_COUNTER(GetFloorDistance);
	float FloorDist = 0.f;
	constexpr float TraceDistance = 1000.f;

	const FVector Start = GetActorLocation();
	const FVector End = Start - FVector(0.f, 0.f, TraceDistance);

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	const bool bHit = GetWorld()->LineTraceSingleByChannel(
		Hit,
		Start,
		End,
		ECC_LockOn,
		Params
	);
	if (bHit)
	{
		FloorDist = Start.Z - Hit.Location.Z;
	}
	else
	{
		FloorDist = 2000.f;
	}
	return FloorDist;
}


DECLARE_CYCLE_STAT(TEXT("HealStamina"), HealStamina, STATGROUP_UK_Character);

void AUK_CharacterBase::HealStamina()
{
	SCOPE_CYCLE_COUNTER(HealStamina);

	TRACE_CPUPROFILER_EVENT_SCOPE(AUK_CharacterBase_HealStamina);
	if (bIsGliding == true)
	{
		bInUseStamina = true;
	}
	if (bIsSprinted == true)
	{
		bInUseStamina = true;
	}

	if (bInUseStamina == false)
	{
		UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
		if (IsValid(ASC) == false)
			return;

		FGameplayEffectSpecHandle SpecHandle =
			ASC->MakeOutgoingSpec(HealStaminaEffect, 1.f, ASC->MakeEffectContext());

		const UUK_PlayerStatusAttributeSet* Attributes = ASC->GetSet<UUK_PlayerStatusAttributeSet>();
		const float HealStaminaAmount = Attributes->GetMaxStamina() * 0.01f;

		const float StaminaAmount = Attributes->GetCurrentStamina();
		if (StaminaAmount >= Attributes->GetMaxStamina())
		{
			return;
		}

		SpecHandle.Data->SetSetByCallerMagnitude(
			UK_GameplayTags::Data::EndBattle::HealStamina,
			HealStaminaAmount
		);

		HealStaminaEffectHandle = ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	}
}

DECLARE_CYCLE_STAT(TEXT("OutOfStamina"), OutOfStamina, STATGROUP_UK_Character);

void AUK_CharacterBase::OutOfStamina()
{
	SCOPE_CYCLE_COUNTER(OutOfStamina);
	OutOfStaminaHandle.Broadcast();
}

#pragma endregion

#pragma  region SaveGame
DECLARE_CYCLE_STAT(TEXT("OnLoadGame"), OnLoadGame, STATGROUP_UK_Character);

void AUK_CharacterBase::OnLoadGame(class UUK_InGameSave* SaveGameObject)
{
	SCOPE_CYCLE_COUNTER(OnLoadGame);

	if (!SaveGameObject) return;

	GetCharacterMovement()->StopMovementImmediately();

	SetActorLocationAndRotation(SaveGameObject->PlayerLocation, SaveGameObject->PlayerRotation, false, nullptr,
	                            ETeleportType::TeleportPhysics);

	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		const UAttributeSet* AS_Base = ASC->GetAttributeSet(UUK_PlayerStatusAttributeSet::StaticClass());
		if (const UUK_PlayerStatusAttributeSet* PSAS = Cast<UUK_PlayerStatusAttributeSet>(AS_Base))
		{
			const_cast<UUK_PlayerStatusAttributeSet*>(PSAS)->ImportStats(SaveGameObject->PlayerStats);
		}
	}

	if (InventoryComponent)
	{
		InventoryComponent->ImportInventory(SaveGameObject->InventoryDate);
	}
}

DECLARE_CYCLE_STAT(TEXT("OnSaveGame"), OnSaveGame, STATGROUP_UK_Character);

void AUK_CharacterBase::OnSaveGame(class UUK_InGameSave* SaveGameObject)
{
	SCOPE_CYCLE_COUNTER(OnSaveGame);

	if (!SaveGameObject) return;

	SaveGameObject->PlayerLocation = GetActorLocation();
	SaveGameObject->PlayerRotation = GetActorRotation();

	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		const UAttributeSet* AS_Base = ASC->GetAttributeSet(UUK_PlayerStatusAttributeSet::StaticClass());
		if (const UUK_PlayerStatusAttributeSet* MyAS = Cast<UUK_PlayerStatusAttributeSet>(AS_Base))
		{
			const_cast<UUK_PlayerStatusAttributeSet*>(MyAS)->ExportStats(SaveGameObject->PlayerStats);
		}
	}
	if (InventoryComponent)
	{
		InventoryComponent->ExportInventory(SaveGameObject->InventoryDate);
	}
}
#pragma endregion

#pragma region GAS
DECLARE_CYCLE_STAT(TEXT("GetAbilitySystemComponent"), GetAbilitySystemComponent, STATGROUP_UK_Character);

UAbilitySystemComponent* AUK_CharacterBase::GetAbilitySystemComponent() const
{
	SCOPE_CYCLE_COUNTER(GetAbilitySystemComponent);

	const AUK_PlayerState* UKPS = Cast<AUK_PlayerState>(GetPlayerState());
	if (!IsValid(UKPS))
		return nullptr;

	return UKPS->GetAbilitySystemComponent();
}

DECLARE_CYCLE_STAT(TEXT("GiveStartupAbilities"), GiveStartupAbilities, STATGROUP_UK_Character);

void AUK_CharacterBase::GiveStartupAbilities()
{
	SCOPE_CYCLE_COUNTER(GiveStartupAbilities);

	if (!IsValid(GetAbilitySystemComponent()))
		return;

	for (const TSubclassOf<UGameplayAbility>& Ability : StartupAbilities)
	{
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(Ability);
		GetAbilitySystemComponent()->GiveAbility(AbilitySpec);
	}
}
#pragma endregion

#pragma region Input
#pragma region MovementFunction

DECLARE_CYCLE_STAT(TEXT("Move"), Move, STATGROUP_UK_Character);

void AUK_CharacterBase::Move(const FInputActionValue& InputActionValue)
{
	SCOPE_CYCLE_COUNTER(Move);

	// bool bIsClimbTrace = ActorTrace();
	// if (bIsClimbTrace == true)
	// {
	// 	if (bIsClimb == false)
	// 	{
	// 		bIsClimb = true;
	// 		Climb(HitResult);
	// 	}
	// 	//const float DeltaTime = GetWorld()->GetDeltaSeconds();
	// 	FVector Normal = HitResult.Normal * -1;
	// 	//FRotator ActorRot = GetActorRotation();
	// 	FRotator TargetRot = FRotationMatrix::MakeFromX(Normal).Rotator();
	//
	// 	// FRotator CurrentRot = FMath::FInterpTo(
	// 	// 	ActorRot,
	// 	// 	TargetRot,
	// 	// 	DeltaTime,
	// 	// 	0.25f
	// 	// );
	// 	SetActorRotation(TargetRot);
	// }
	// if (bIsClimbTrace == false)
	// {
	// 	bIsClimb = false;
	// }

	const FVector2D MovementVector = InputActionValue.Get<FVector2D>();
	const FRotator MovementRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);

	FVector ForwardDirection;
	FVector RightDirection;

	// GetCharacterMovement()->MovementMode;
	// switch (GetCharacterMovement()->MovementMode)
	// {
	// case MOVE_Custom:
	// 	switch ((ECustomMovementMode)GetCharacterMovement()->CustomMovementMode)
	// 	{
	// 	// case ECustomMovementMode::CMOVE_Glide:
	// 	// 	ForwardDirection = Controller->GetControlRotation().Vector();
	// 	// 	RightDirection = FRotationMatrix(MovementRotation).GetUnitAxis(EAxis::Y);
	// 	// 	break;
	//
	// 	case ECustomMovementMode::CMOVE_Climb:
	// 		ForwardDirection = FVector::UpVector;
	// 		RightDirection = FRotationMatrix(MovementRotation).GetUnitAxis(EAxis::Y);
	// 		break;
	// 	}
	// 	break;
	// default:
	//}
	// if (bIsClimbTrace == true)
	// {
	// 	ForwardDirection = FVector::UpVector;
	// 	RightDirection = FRotationMatrix(MovementRotation).GetUnitAxis(EAxis::Y);
	// }
	// else
	// {
	// }
	ForwardDirection = FRotationMatrix(MovementRotation).GetUnitAxis(EAxis::X);
	RightDirection = FRotationMatrix(MovementRotation).GetUnitAxis(EAxis::Y);
	if (FMath::IsNearlyZero(MovementVector.X) == false)
	{
		if (bIsGliding == true && MovementVector.X > 0.f)
		{
			AddMovementInput(ForwardDirection, MovementVector.X);
		}
		else if (bIsGliding == false)
		{
			AddMovementInput(ForwardDirection, MovementVector.X);
		}
	}

	if (FMath::IsNearlyZero(MovementVector.Y) == false)
	{
		AddMovementInput(RightDirection, MovementVector.Y);
	}
}

DECLARE_CYCLE_STAT(TEXT("Look"), Look, STATGROUP_UK_Character);

void AUK_CharacterBase::Look(const FInputActionValue& InputActionValue)
{
	SCOPE_CYCLE_COUNTER(Look);
	const FVector2D LookAxisVector = InputActionValue.Get<FVector2D>();

	if (FMath::IsNearlyZero(LookAxisVector.Y) == false)
	{
		AddControllerPitchInput(LookAxisVector.Y);
	}
	if (bIsLock == true)
	{
		return;
	}
	if (FMath::IsNearlyZero(LookAxisVector.X) == false)
	{
		AddControllerYawInput(LookAxisVector.X);
	}
}

DECLARE_CYCLE_STAT(TEXT("ZoomIn"), ZoomIn, STATGROUP_UK_Character);

void AUK_CharacterBase::ZoomIn()
{
	SCOPE_CYCLE_COUNTER(ZoomIn);

	if (!IsValid(SpringArmComp))
	{
		return;
	}
	const float DeltaTime = GetWorld()->GetDeltaSeconds();

	constexpr float Target = 70.f;
	SpringArmComp->TargetArmLength = FMath::FInterpTo(
		SpringArmComp->TargetArmLength,
		Target,
		DeltaTime,
		12.f
	);
}

DECLARE_CYCLE_STAT(TEXT("ZoomOut"), ZoomOut, STATGROUP_UK_Character);

void AUK_CharacterBase::ZoomOut()
{
	SCOPE_CYCLE_COUNTER(ZoomOut);

	if (!IsValid(SpringArmComp))
	{
		return;
	}
	const float DeltaTime = GetWorld()->GetDeltaSeconds();

	constexpr float Target = 450.f;
	SpringArmComp->TargetArmLength = FMath::FInterpTo(
		SpringArmComp->TargetArmLength,
		Target,
		DeltaTime,
		12.f
	);
}

DECLARE_CYCLE_STAT(TEXT("UK Character Logic"), LightAttack, STATGROUP_UK_Character);

void AUK_CharacterBase::LightAttack()
{
	SCOPE_CYCLE_COUNTER(LightAttack);
	float Dist = 0.f;
	FGameplayTagContainer Container;
	if (GetCharacterMovement()->IsFalling() == true)
	{
		Dist = GetFloorDistance();
		UE_LOG(LogTemp, Display, TEXT("%f"), Dist);
		InputType = EInputMode::Air;
		UE_LOG(LogTemp, Display, TEXT("%s"), *GetName());
		if (Dist > 140)
		{
			Container.AddTag(UK_GameplayTags::Action::AirAttack);
			GetAbilitySystemComponent()->TryActivateAbilitiesByTag(Container);
		}
	}
	else
	{
		InputType = EInputMode::Light;
		Container.AddTag(UK_GameplayTags::Action::LightAttack);
		GetAbilitySystemComponent()->TryActivateAbilitiesByTag(Container);
	}
}

DECLARE_CYCLE_STAT(TEXT("HeavyAttack"), HeavyAttack, STATGROUP_UK_Character);

void AUK_CharacterBase::HeavyAttack()
{
	SCOPE_CYCLE_COUNTER(HeavyAttack);

	InputType = EInputMode::Heavy;
	FGameplayTagContainer Container;
	Container.AddTag(UK_GameplayTags::Action::HeavyAttack);
	GetAbilitySystemComponent()->TryActivateAbilitiesByTag(Container);
}

DECLARE_CYCLE_STAT(TEXT("NormalSkill"), NormalSkill, STATGROUP_UK_Character);

void AUK_CharacterBase::NormalSkill()
{
	SCOPE_CYCLE_COUNTER(NormalSkill);

	InputType = EInputMode::NormalSkill;
	FGameplayTagContainer Container;
	Container.AddTag(UK_GameplayTags::Input::NomalSkill);
	GetAbilitySystemComponent()->TryActivateAbilitiesByTag(Container);
}

DECLARE_CYCLE_STAT(TEXT("UltimateSkill"), UltimateSkill, STATGROUP_UK_Character);

void AUK_CharacterBase::UltimateSkill()
{
	SCOPE_CYCLE_COUNTER(UltimateSkill);

	InputType = EInputMode::UltimateSkill;
	FGameplayTagContainer Container;
	Container.AddTag(UK_GameplayTags::Input::UltimateSkill);
	GetAbilitySystemComponent()->TryActivateAbilitiesByTag(Container);
}

DECLARE_CYCLE_STAT(TEXT("Parry"), Parry, STATGROUP_UK_Character);

void AUK_CharacterBase::Parry()
{
	SCOPE_CYCLE_COUNTER(Parry);

	InputType = EInputMode::Parry;
	FGameplayTagContainer Container;
	Container.AddTag(UK_GameplayTags::Action::Parry);
	GetAbilitySystemComponent()->TryActivateAbilitiesByTag(Container);
}

DECLARE_CYCLE_STAT(TEXT("ToggleMouse"), ToggleMouse, STATGROUP_UK_Character);

void AUK_CharacterBase::ToggleMouse()
{
	SCOPE_CYCLE_COUNTER(ToggleMouse);

	if (PC == nullptr)
		return;

	PC->ToggleMouseCursor();
}

DECLARE_CYCLE_STAT(TEXT("Interaction"), Interaction, STATGROUP_UK_Character);

void AUK_CharacterBase::Interaction()
{
	SCOPE_CYCLE_COUNTER(Interaction);

	if (InteractionComp)
	{
		InteractionComp->TryInteract();
	}
	UE_LOG(LogTemp, Log, TEXT("상호 작용 시도"));
}

DECLARE_CYCLE_STAT(TEXT("Setting"), Setting, STATGROUP_UK_Character);

void AUK_CharacterBase::Setting()
{
	SCOPE_CYCLE_COUNTER(Setting);

	if (PC == nullptr)
		return;

	PC->OpenSettingAndCloseOtherUI();
}

DECLARE_CYCLE_STAT(TEXT("Inventory"), Inventory, STATGROUP_UK_Character);

void AUK_CharacterBase::Inventory()
{
	if ( GEngine )
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			2.0f,
			FColor::Green,
			TEXT("B 누름")
		);
	}
	if ( PC == nullptr ) return;

	PC->Inventory_UI();
}

DECLARE_CYCLE_STAT(TEXT("WeaponCrafting"), WeaponCrafting, STATGROUP_UK_Character);

void AUK_CharacterBase::WeaponCrafting()
{
	if ( GEngine )
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			2.0f,
			FColor::Green,
			TEXT("U 누름")
		);
	}
	if ( PC == nullptr ) return;

	PC->WeaponCrafting_UI();
}

DECLARE_CYCLE_STAT(TEXT("Esc"), Esc, STATGROUP_UK_Character);

void AUK_CharacterBase::Esc()
{
	if ( GEngine )
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			2.0f,
			FColor::Green,
			TEXT("Esc 누름")
		);
	}

	if ( PC == nullptr ) return;
	if ( PC->CloseOpenWidget() ) return;
}

#pragma endregion

#pragma region LockOn

DECLARE_CYCLE_STAT(TEXT("LockON"), LockON, STATGROUP_UK_Character);

void AUK_CharacterBase::LockON()
{
	SCOPE_CYCLE_COUNTER(LockON);

	TRACE_CPUPROFILER_EVENT_SCOPE(AUK_CharacterBase_LockOn);

	if (bIsLock == false)
	{
		bIsLock = true;
		bUseControllerRotationYaw = true;
		GetCharacterMovement()->bOrientRotationToMovement = false;
		if (GetWorld()->GetTimerManager().IsTimerActive(LockOnTimer) == false)
		{
			GetWorld()->GetTimerManager().SetTimer(
				LockOnTimer,
				this,
				&AUK_CharacterBase::LockONTick,
				0.01f,
				true
			);
		}
	}
}

DECLARE_CYCLE_STAT(TEXT("LockONToggle"), LockONToggle, STATGROUP_UK_Character);

void AUK_CharacterBase::LockONToggle()
{
	SCOPE_CYCLE_COUNTER(LockONToggle);

	TRACE_CPUPROFILER_EVENT_SCOPE(AUK_CharacterBase_LockOnToggle);
	if (bIsLock == false)
	{
		AUK_PlayerController* UKPC = Cast<AUK_PlayerController>(GetController());
		if (IsValid(UKPC) == false)
			return;
		FVector Start;
		FRotator CameraRot;
		constexpr float CapsuleRadius = 50.f;
		// 카메라부터 카메라가 보는 방향으로 트레이스 실시
		UKPC->GetPlayerViewPoint(Start, CameraRot);
		FVector ForwardVector = CameraRot.Vector(); /*카메라의 방향성*/

		float TraceDistance = 1000.f;
		FVector End = Start + (ForwardVector * TraceDistance);

		FCollisionQueryParams Params;
		Params.AddIgnoredActor(this);
		FCollisionShape CollisionShape = FCollisionShape::MakeSphere(CapsuleRadius);

		TArray<FHitResult> LockOnResult;

		bool bHit = GetWorld()->SweepMultiByChannel(
			LockOnResult,
			Start,
			End,
			FQuat::Identity,
			ECC_LockOn, /*추후에 카메라 전용 트레이스 채널로 변경 요망*/
			CollisionShape,
			Params
		);
		FColor DrawColor = bHit ? FColor::Green : FColor::Red;

		FQuat CapsuleRot = FRotationMatrix::MakeFromZ(Start - End).ToQuat();
		DrawDebugCapsule(
			GetWorld(),
			(Start + End) / 2,
			(End - Start).Size(),
			CapsuleRadius,
			CapsuleRot,
			DrawColor,
			false,
			1.f
		);
		if (bHit)
		{
			for (const FHitResult& Hit : LockOnResult)
			{
				if (TObjectPtr<AAIMonsterBase> Monster = Cast<AAIMonsterBase>(Hit.GetActor()))
				{
					AddTarget(Monster);
				}
			}
			if (LockOnList.Num() > 0)
			{
				bIsLock = true;
				bUseControllerRotationYaw = true;
				GetCharacterMovement()->bOrientRotationToMovement = false;
				if (GetWorld()->GetTimerManager().IsTimerActive(LockOnTimer) == false)
				{
					GetWorld()->GetTimerManager().SetTimer(
						LockOnTimer,
						this,
						&AUK_CharacterBase::LockONTick,
						1.f,
						true
					);
				}
			}
		}
	}
	else
	{
		bIsLock = false;
		bUseControllerRotationYaw = false;
		GetCharacterMovement()->bOrientRotationToMovement = true;
		GetWorld()->GetTimerManager().ClearTimer(LockOnTimer);
		LockOnList.Reset();

		LockOnTimer.Invalidate();
	}
}

DECLARE_CYCLE_STAT(TEXT("LockONTick"), LockONTick, STATGROUP_UK_Character);

void AUK_CharacterBase::LockONTick()
{
	SCOPE_CYCLE_COUNTER(LockONTick);

	TRACE_CPUPROFILER_EVENT_SCOPE(AUK_CharacterBase_LockONTick);

	if (LockOnList.IsEmpty() == false)
	{
		if (const int32 Size = LockOnList.Num(); Size <= LockOnIndex)
		{
			LockOnIndex = 0;
		}
		const AAIMonsterBase* Monster = LockOnList[LockOnIndex];
		if (IsValid(Monster) == false)
		{
			LockOnList.RemoveAtSwap(LockOnIndex);
			return;
		}

		if (Monster->IsDead() == false)
		{
			if (const float Distance = FVector::Dist(GetActorLocation(), Monster->GetActorLocation()); Distance >
				MaxLockDistance)
			{
				bIsLock = false;
				GetWorld()->GetTimerManager().ClearTimer(LockOnTimer);
				LockOnList.Reset();
				LockOnTimer.Invalidate();
				bUseControllerRotationYaw = false;
				GetCharacterMovement()->bOrientRotationToMovement = true;
				return;
			}
			const float DeltaTime = GetWorld()->GetDeltaSeconds();
			const FVector Start = GetActorLocation();
			const FVector End = Monster->GetActorLocation();
			const FRotator Target = UKismetMathLibrary::FindLookAtRotation(Start, End);

			const FRotator NowRot = GetController()->GetControlRotation();

			FRotator Desired = NowRot;
			Desired.Yaw = Target.Yaw;
			Desired.Roll = 0.f;

			const FRotator NewRot = FMath::RInterpTo(
				NowRot,
				Desired,
				DeltaTime,
				12.f
			);

			GetController()->SetControlRotation(NewRot);
		}
		else if (Monster->IsDead() == true)
		{
			LockOnList.RemoveAtSwap(LockOnIndex);
		}
	}
	else
	{
		bIsLock = false;
		GetWorld()->GetTimerManager().ClearTimer(LockOnTimer);
		LockOnList.Reset();
		LockOnTimer.Invalidate();
		bUseControllerRotationYaw = false;
		GetCharacterMovement()->bOrientRotationToMovement = true;
		return;
	}
}

DECLARE_CYCLE_STAT(TEXT("AddTarget"), AddTarget, STATGROUP_UK_Character);

void AUK_CharacterBase::AddTarget(const TObjectPtr<AAIMonsterBase> Monster)
{
	SCOPE_CYCLE_COUNTER(AddTarget);

	LockOnList.AddUnique(Monster);
}

#pragma endregion

#pragma region Gliding

DECLARE_CYCLE_STAT(TEXT("StartGliding"), StartGliding, STATGROUP_UK_Character);

bool AUK_CharacterBase::StartGliding()
{
	SCOPE_CYCLE_COUNTER(StartGliding);

	if (GetCharacterMovement()->IsFalling() == false)
		return false;
	if (bIsGliding == true)
		return false;
	if (GetFloorDistance() < 220.f)
	{
		return false;
	}
	GetCharacterMovement()->StopMovementImmediately();
	FVector Vel = GetCharacterMovement()->Velocity;
	Vel.Z = -GlideFallSpeed;
	GetCharacterMovement()->GravityScale = 0.f;
	GetCharacterMovement()->AirControl = 0.8;
	GetCharacterMovement()->Velocity = Vel;
	bIsGliding = true;

	bInUseStamina = true;

	//GetCharacterMovement()->SetMovementMode(MOVE_Custom, (uint8)ECustomMovementMode::CMOVE_Gliding);
	return true;
}

DECLARE_CYCLE_STAT(TEXT("EndGliding"), EndGliding, STATGROUP_UK_Character);

void AUK_CharacterBase::EndGliding()
{
	SCOPE_CYCLE_COUNTER(EndGliding);

	bIsGliding = false;
	// if (GetCharacterMovement()->CurrentFloor.IsWalkableFloor() == false)
	// {
	// 	GetCharacterMovement()->SetMovementMode(MOVE_Falling);
	// }
	// else
	// {
	// 	GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	// }

	GetCharacterMovement()->GravityScale = DefualtGravity;
	GetCharacterMovement()->AirControl = DefualtAirControl;
	FTimerHandle EndGlidingTimer;
	GetWorldTimerManager().SetTimer(
		EndGlidingTimer,
		[this]()
		{
			bInUseStamina = false;
		},
		1.f,
		false
	);
}

#pragma endregion

#pragma region Climb
DECLARE_CYCLE_STAT(TEXT("ActorTrace"), ActorTrace, STATGROUP_UK_Character);

bool AUK_CharacterBase::ActorTrace()
{
	SCOPE_CYCLE_COUNTER(ActorTrace);

	FVector Start = SkeletalMeshComp->GetSocketLocation("LookAt");
	FVector End = Start + (GetActorForwardVector() * TraceDist);

	bool Hit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		Start,
		End,
		ECC_Visibility);

	return Hit;
	// DrawDebugLine(
	// 	GetWorld(),
	// 	Start,
	// 	End,
	// 	FColor::Red,
	// 	false,
	// 	1.0f,
	// 	0,
	// 	2.0f
	// );
	// UCustomCharacterMovementComponent* Movement = Cast<UCustomCharacterMovementComponent>(GetCharacterMovement());
	// Movement->bIsClimbingSurface = bWallDetected;
	// if (bWallDetected)
	// {
	// 	if (IsValid(Movement))
	// 	{
	// 		Movement->CurrentClimbNormal = HitResult.ImpactNormal;
	// 	}
	// }
}

DECLARE_CYCLE_STAT(TEXT("Climb"), Climb, STATGROUP_UK_Character);

void AUK_CharacterBase::Climb(FHitResult& Hit)
{
	SCOPE_CYCLE_COUNTER(Climb);

	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	MoveComp->SetMovementMode(MOVE_Flying);
	MoveComp->bOrientRotationToMovement = false;

	FVector Normal = Hit.Normal * -1.f;
	FVector Location = Hit.ImpactPoint + Hit.Normal * GetCapsuleComponent()->GetUnscaledCapsuleRadius();
	//(GetCapsuleComponent()->GetUnscaledCapsuleRadius()) * Hit.Normal;
	FRotator Rot = FRotationMatrix::MakeFromX(Normal).Rotator();

	FLatentActionInfo LatentInfo;
	LatentInfo.CallbackTarget = this;

	UKismetSystemLibrary::MoveComponentTo(
		GetCapsuleComponent(),
		Location,
		Rot,
		false,
		false,
		0.2f,
		false,
		EMoveComponentAction::Move,
		LatentInfo
	);
}

DECLARE_CYCLE_STAT(TEXT("StartSprintCost"), StartSprintCost, STATGROUP_UK_Character);

void AUK_CharacterBase::StartSprintCost()
{
	SCOPE_CYCLE_COUNTER(StartSprintCost);

	bIsSprinted = true;
	bInUseStamina = true;
	FGameplayTagContainer TagContainer;
	TagContainer.AddTag(UK_GameplayTags::Input::Sprint);
	GetAbilitySystemComponent()->TryActivateAbilitiesByTag(TagContainer);
}

DECLARE_CYCLE_STAT(TEXT("EndSprintCost"), EndSprintCost, STATGROUP_UK_Character);

void AUK_CharacterBase::EndSprintCost()
{
	SCOPE_CYCLE_COUNTER(EndSprintCost);

	bIsSprinted = false;
	FTimerHandle EndSprintTimer;
	GetWorldTimerManager().SetTimer(
		EndSprintTimer,
		[this]()
		{
			bInUseStamina = false;
		},
		1.f,
		false
	);
}


#pragma endregion

#pragma endregion

#pragma region Weapon

DECLARE_CYCLE_STAT(TEXT("ChangeWeaponStat"), ChangeWeaponStat, STATGROUP_UK_Character);

void AUK_CharacterBase::ChangeWeaponStat(const FUK_WeaponItemData* WeaponStat)
{
	SCOPE_CYCLE_COUNTER(ChangeWeaponStat);

	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (IsValid(ASC) == false)
		return;

	FGameplayEffectSpecHandle SpecHandle =
		ASC->MakeOutgoingSpec(WeaponStatEffect, 1.f, ASC->MakeEffectContext());
	// 기존 무기 효과 제거
	if (WeaponEffectHandle.IsValid())
	{
		ASC->RemoveActiveGameplayEffect(WeaponEffectHandle);
		WeaponEffectHandle.Invalidate();
	}

	if (WeaponStat == nullptr)
		return;


	SpecHandle.Data->SetSetByCallerMagnitude(
		UK_GameplayTags::Data::WeaponStat::ExtraAttackPower,
		WeaponStat->ExtraAttackPower
	);

	WeaponEffectHandle = ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
}

DECLARE_CYCLE_STAT(TEXT("EquipWeapon"), EquipWeapon, STATGROUP_UK_Character);

void AUK_CharacterBase::EquipWeapon(FGameplayTag NewWeapon)
{
	SCOPE_CYCLE_COUNTER(EquipWeapon);

	if (NewWeapon == UK_GameplayTags::Weapon::WeaponRoot)
	{
		RightHandWeaponComponent->SetSkeletalMesh(nullptr);
		LeftHandWeaponComponent->SetSkeletalMesh(nullptr);
		NowWeapon = nullptr;
		return;
	}
	UUK_StatusAnimData* Weapon = WeaponList->FindAnimsDataAssetByTag(NewWeapon);
	NowWeapon = Weapon;
	FWeaponStatus WeaponStatus = Weapon->FindAnimsDataAssetByType(NewWeapon);
	if (IsValid(WeaponStatus.RightHandWeapon))
	{
		RightHandWeaponComponent->SetSkeletalMesh(WeaponStatus.RightHandWeapon);

		RightHandWeaponComponent->SetRelativeLocation(WeaponStatus.RightLocationOffset);
		RightHandWeaponComponent->SetRelativeRotation(WeaponStatus.RightRotationOffset);
	}
	else
	{
		RightHandWeaponComponent->SetSkeletalMesh(nullptr);
	}
	if (IsValid(WeaponStatus.LeftHandWeapon))
	{
		LeftHandWeaponComponent->SetSkeletalMesh(WeaponStatus.LeftHandWeapon);

		LeftHandWeaponComponent->SetRelativeLocation(WeaponStatus.LeftLocationOffset);
		LeftHandWeaponComponent->SetRelativeRotation(WeaponStatus.LeftRotationOffset);
	}
	else
	{
		LeftHandWeaponComponent->SetSkeletalMesh(nullptr);
	}
}

DECLARE_CYCLE_STAT(TEXT("SlotWeaponOne"), SlotWeaponOne, STATGROUP_UK_Character);

void AUK_CharacterBase::SlotWeaponOne()
{
	SCOPE_CYCLE_COUNTER(SlotWeaponOne);

	if (WeaponSlotIndex == 1)
	{
		WeaponSlotIndex = 0;
		ChangeWeaponStat(nullptr);
		ChangedAttribute(ECharacterAttribute::None);
		EquipWeapon(UK_GameplayTags::Weapon::WeaponRoot);
		return;
	}
	WeaponSlotIndex = 1;
	FGameplayTagContainer Container;
	Container.AddTag(UK_GameplayTags::Action::Swap1);
	GetAbilitySystemComponent()->TryActivateAbilitiesByTag(Container);
}

DECLARE_CYCLE_STAT(TEXT("SlotWeaponTwo"), SlotWeaponTwo, STATGROUP_UK_Character);

void AUK_CharacterBase::SlotWeaponTwo()
{
	SCOPE_CYCLE_COUNTER(SlotWeaponTwo);

	if (WeaponSlotIndex == 2)
	{
		WeaponSlotIndex = 0;
		ChangeWeaponStat(nullptr);
		ChangedAttribute(ECharacterAttribute::None);
		EquipWeapon(UK_GameplayTags::Weapon::WeaponRoot);
		return;
	}
	WeaponSlotIndex = 2;
	FGameplayTagContainer Container;
	Container.AddTag(UK_GameplayTags::Action::Swap2);
	GetAbilitySystemComponent()->TryActivateAbilitiesByTag(Container);
}

DECLARE_CYCLE_STAT(TEXT("SlotWeaponThree"), SlotWeaponThree, STATGROUP_UK_Character);

void AUK_CharacterBase::SlotWeaponThree()
{
	SCOPE_CYCLE_COUNTER(SlotWeaponThree);

	if (WeaponSlotIndex == 3)
	{
		WeaponSlotIndex = 0;
		ChangeWeaponStat(nullptr);
		ChangedAttribute(ECharacterAttribute::None);
		EquipWeapon(UK_GameplayTags::Weapon::WeaponRoot);
		return;
	}
	WeaponSlotIndex = 3;

	FGameplayTagContainer Container;
	Container.AddTag(UK_GameplayTags::Action::Swap3);
	GetAbilitySystemComponent()->TryActivateAbilitiesByTag(Container);
}

DECLARE_CYCLE_STAT(TEXT("SwapWeapon"), SwapWeapon, STATGROUP_UK_Character);

void AUK_CharacterBase::SwapWeapon(int32 Index)
{
	SCOPE_CYCLE_COUNTER(SwapWeapon);

	if (WeaponSlotIndex != Index + 1)
		return;
	if (IsValid(WeaponDataTable) == false)
	{
		UE_LOG(LogTemp, Display, TEXT("ItmeDataTable is Nullptr"));
		return;
	}
	FInventorySlot* WeaponSlot = InventoryComponent->FindWeaponSlotbyIndex(Index);
	if (WeaponSlot->isEmpty())
	{
		WeaponSlotIndex = 0;
		ChangeWeaponStat(nullptr);
		ChangedAttribute(ECharacterAttribute::None);
		EquipWeapon(UK_GameplayTags::Weapon::WeaponRoot);
		return;
	}
	const FUK_WeaponItemData* ItemData = WeaponDataTable->FindRow<FUK_WeaponItemData>(
		WeaponSlot->ItemID, TEXT("AUK_CharacterBase::SwapWeapon"));
	if (ItemData == nullptr)
	{
		WeaponSlotIndex = 0;
		ChangeWeaponStat(nullptr);
		ChangedAttribute(ECharacterAttribute::None);
		EquipWeapon(UK_GameplayTags::Weapon::WeaponRoot);
		return;
	}
	ChangeWeaponStat(ItemData);
	ChangedAttribute(ItemData->WeaponAttribute);
	EquipWeapon(ItemData->ItemTag);
}

#pragma endregion

#pragma region Battle

DECLARE_CYCLE_STAT(TEXT("StopJumpAndFly"), StopJumpAndFly, STATGROUP_UK_Character);

void AUK_CharacterBase::StopJumpAndFly()
{
	SCOPE_CYCLE_COUNTER(StopJumpAndFly);

	bIsFry = true;
	UCharacterMovementComponent* PlayerMovement = GetCharacterMovement();
	StopJumping();
	PlayerMovement->Velocity = FVector::ZeroVector;
	PlayerMovement->GravityScale = 0.f;

	PlayerMovement->SetJumpAllowed(false);
}

DECLARE_CYCLE_STAT(TEXT("EndComboAttack"), EndComboAttack, STATGROUP_UK_Character);

void AUK_CharacterBase::EndComboAttack()
{
	SCOPE_CYCLE_COUNTER(EndComboAttack);

	if (bIsFry == false)
		return;
	UCharacterMovementComponent* PlayerMovement = GetCharacterMovement();
	PlayerMovement->GravityScale = DefualtGravity;
	PlayerMovement->SetMovementMode(EMovementMode::MOVE_Walking);
	PlayerMovement->SetJumpAllowed(true);
	bIsFry = false;
}

DECLARE_CYCLE_STAT(TEXT("Dead"), Dead, STATGROUP_UK_Character);

void AUK_CharacterBase::Dead()
{
	SCOPE_CYCLE_COUNTER(Dead);

	if (GetAbilitySystemComponent()->HasMatchingGameplayTag(UK_GameplayTags::Status::Dead))
		return;

	UE_LOG(LogTemp, Display, TEXT("Is Player Dead"));
	GetCharacterMovement()->DisableMovement();
	GetController()->SetIgnoreMoveInput(true);
	GetController()->SetIgnoreLookInput(true);
	GetAbilitySystemComponent()->AddLooseGameplayTag(UK_GameplayTags::Status::Dead);

	FOnMontageEnded EndDelegate;
	if (DeathMontage)
	{
		PlayAnimMontage(DeathMontage);
	}
	OnDead.Broadcast();
}

DECLARE_CYCLE_STAT(TEXT("Resurrection"), Resurrection, STATGROUP_UK_Character);

void AUK_CharacterBase::Resurrection()
{
	SCOPE_CYCLE_COUNTER(Resurrection);
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (IsValid(ASC) == false)
		return;
	const UUK_PlayerStatusAttributeSet* Attributes = ASC->GetSet<UUK_PlayerStatusAttributeSet>();
	if (Attributes == nullptr)
		return;

	ASC->SetNumericAttributeBase(
		UUK_PlayerStatusAttributeSet::GetHealthAttribute(),
		100.f
	);
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && DeathMontage)
	{
		if (AnimInstance->Montage_IsPlaying(DeathMontage))
		{
			AnimInstance->Montage_Stop(0.2f, DeathMontage);
		}
	}
}

DECLARE_CYCLE_STAT(TEXT("StartBattle"), StartBattle, STATGROUP_UK_Character);

void AUK_CharacterBase::StartBattle()
{
	SCOPE_CYCLE_COUNTER(HeavyAttack);

	bInBattle = true;
	if (GetWorldTimerManager().IsTimerActive(EndBattleTimerHandle))
	{
		GetWorldTimerManager().ClearTimer(EndBattleTimerHandle);
	}

	// 사운드 매니저를 찾아서 전투 상태를 True로 변경
	if (SoundManager)
	{
		SoundManager->SetCombatState(true);
	}
}

DECLARE_CYCLE_STAT(TEXT("EndBattle"), EndBattle, STATGROUP_UK_Character);

void AUK_CharacterBase::EndBattle()
{
	SCOPE_CYCLE_COUNTER(EndBattle);

	bInBattle = false;
	
	// 사운드 매니저를 찾아서 전투 상태를 False로 변경
	if (SoundManager)
	{
		SoundManager->SetCombatState(false);
	}

	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (IsValid(ASC) == false)
		return;

	FGameplayEffectSpecHandle SpecHandle =
		ASC->MakeOutgoingSpec(EndBattleEffect, 1.f, ASC->MakeEffectContext());

	const UUK_PlayerStatusAttributeSet* Attributes = ASC->GetSet<UUK_PlayerStatusAttributeSet>();

	if ((Attributes->GetHealth() >= Attributes->GetMaxHealth()) && (Attributes->GetCurrentMp() >= Attributes->
		GetMaxMp()))
		return;

	if (Attributes->GetHealth() < Attributes->GetMaxHealth())
	{
		const float HealHPAmount = Attributes->GetMaxHealth() * 0.05f;
		SpecHandle.Data->SetSetByCallerMagnitude(
			UK_GameplayTags::Data::EndBattle::HealHP,
			HealHPAmount
		);
	}

	if (Attributes->GetCurrentMp() < Attributes->GetMaxMp())
	{
		const float HealMPAmount = Attributes->GetMaxMp() * 0.05f;
		SpecHandle.Data->SetSetByCallerMagnitude(
			UK_GameplayTags::Data::EndBattle::HealMP,
			HealMPAmount
		);
	}

	EndBattleEffectHandle = ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
}

DECLARE_CYCLE_STAT(TEXT("HandleLevelUp"), HandleLevelUp, STATGROUP_UK_Character);

void AUK_CharacterBase::HandleLevelUp()
{
	SCOPE_CYCLE_COUNTER(HandleLevelUp);

	// 1. VFX 재생
	if (LevelUpVFX && GetMesh())
	{
		UNiagaraFunctionLibrary::SpawnSystemAttached(
					LevelUpVFX,               // 재생할 이펙트
					GetMesh(),                // 부착할 대상 (캐릭터 메쉬)
					FName("spine_04"),        // 따라다닐 소켓/본 이름
					FVector(0.f, 0.f, 0.f),   // 위치 오프셋 (필요시 수정)
					FRotator::ZeroRotator,    // 회전 오프셋
					EAttachLocation::SnapToTarget, // 소켓 위치에 딱 붙이기
					true                      // 이펙트 종료 시 자동 제거
				);
	}

	// 2. 사운드 재생
	if (LevelUpSound)
	{
		UGameplayStatics::PlaySound2D(GetWorld(), LevelUpSound);
	}
}

#pragma endregion

#pragma region FindMonsterHPBar

DECLARE_CYCLE_STAT(TEXT("UpdateMonsterDetection"), UpdateMonsterDetection, STATGROUP_UK_Character);

void AUK_CharacterBase::UpdateMonsterDetection()
{
	SCOPE_CYCLE_COUNTER(UpdateMonsterDetection);

	if (!IsLocallyControlled())
		return;
	TArray<FOverlapResult> Results;
	FCollisionShape Sphere = FCollisionShape::MakeSphere(DetectRadius);
	GetWorld()->OverlapMultiByObjectType(Results, GetActorLocation(), FQuat::Identity,
	                                     FCollisionObjectQueryParams(ECC_Pawn), Sphere);
	if (bDrawDetectRadius)
	{
		DrawDebugSphere(GetWorld(), GetActorLocation(), DetectRadius, 32, FColor::Green, false, 0.31f);
	}
	TSet<AAIMonsterBase*> NewSet;
	// overlap이 되는 것들의 data result 결과들
	for (const FOverlapResult& Result : Results)
	{
		AActor* OverlappedActor = Result.OverlapObjectHandle.FetchActor();
		if (AAIMonsterBase* Monster = Cast<AAIMonsterBase>(OverlappedActor))
		{
			NewSet.Add(Monster);
			if (!NearbyMonsters.Contains(Monster))
			{
				Monster->ShowHPBar();
			}
		}
	}
	if (NewSet.Num() > 0)
	{
		StartBattle();
	}
	else
	{
		if (GetWorldTimerManager().IsTimerActive(EndBattleTimerHandle) == false)
		{
			GetWorldTimerManager().SetTimer(
				EndBattleTimerHandle,
				this,
				&AUK_CharacterBase::EndBattle,
				5.f,
				false
			);
		}
	}
	// 범위가 벗어났는지 확인 
	for (AAIMonsterBase* OldMonster : NearbyMonsters)
	{
		if (IsValid(OldMonster) && !NewSet.Contains(OldMonster))
		{
			OldMonster->HideHPBar();
		}
	}
	NearbyMonsters = NewSet;
}

#pragma endregion
