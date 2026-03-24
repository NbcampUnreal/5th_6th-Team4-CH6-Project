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
DECLARE_CYCLE_STAT(TEXT("UK Character Logic"), Constructor, STATGROUP_UK_Character);

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
void AUK_CharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	//UpdateMovementState();
}

void AUK_CharacterBase::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
}

void AUK_CharacterBase::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);
	FGameplayEventData EventData;
	EventData.EventTag = UK_GameplayTags::Action::DropAttack;
	GetAbilitySystemComponent()->HandleGameplayEvent(EventData.EventTag, &EventData);
}

void AUK_CharacterBase::ChangedAttribute(ECharacterAttribute NewAttribute)
{
	Attribute = NewAttribute;
	OnChangedAttribute.Broadcast(NewAttribute);
}

void AUK_CharacterBase::UpdateMovementState()
{
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

DECLARE_CYCLE_STAT(TEXT("UK Character Logic"), BeginPlay, STATGROUP_UK_Character);

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

void AUK_CharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
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
DECLARE_CYCLE_STAT(TEXT("UK Character Logic"), GetFloorDistance, STATGROUP_UK_Character);

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


DECLARE_CYCLE_STAT(TEXT("UK Character Logic"), HealStamina, STATGROUP_UK_Character);
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

void AUK_CharacterBase::OutOfStamina()
{
	OutOfStaminaHandle.Broadcast();
}

#pragma endregion

#pragma  region SaveGame

void AUK_CharacterBase::OnLoadGame(class UUK_InGameSave* SaveGameObject)
{
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

void AUK_CharacterBase::OnSaveGame(class UUK_InGameSave* SaveGameObject)
{
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

UAbilitySystemComponent* AUK_CharacterBase::GetAbilitySystemComponent() const
{
	const AUK_PlayerState* UKPS = Cast<AUK_PlayerState>(GetPlayerState());
	if (!IsValid(UKPS))
		return nullptr;

	return UKPS->GetAbilitySystemComponent();
}

void AUK_CharacterBase::GiveStartupAbilities()
{
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

void AUK_CharacterBase::Move(const FInputActionValue& InputActionValue)
{
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

void AUK_CharacterBase::Look(const FInputActionValue& InputActionValue)
{
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

void AUK_CharacterBase::ZoomIn()
{
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

void AUK_CharacterBase::ZoomOut()
{
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

void AUK_CharacterBase::HeavyAttack()
{
	InputType = EInputMode::Heavy;
	FGameplayTagContainer Container;
	Container.AddTag(UK_GameplayTags::Action::HeavyAttack);
	GetAbilitySystemComponent()->TryActivateAbilitiesByTag(Container);
}

void AUK_CharacterBase::NormalSkill()
{
	InputType = EInputMode::NormalSkill;
	FGameplayTagContainer Container;
	Container.AddTag(UK_GameplayTags::Input::NomalSkill);
	GetAbilitySystemComponent()->TryActivateAbilitiesByTag(Container);
}

void AUK_CharacterBase::UltimateSkill()
{
	InputType = EInputMode::UltimateSkill;
	FGameplayTagContainer Container;
	Container.AddTag(UK_GameplayTags::Input::UltimateSkill);
	GetAbilitySystemComponent()->TryActivateAbilitiesByTag(Container);
}

void AUK_CharacterBase::Parry()
{
	InputType = EInputMode::Parry;
	FGameplayTagContainer Container;
	Container.AddTag(UK_GameplayTags::Action::Parry);
	GetAbilitySystemComponent()->TryActivateAbilitiesByTag(Container);
}

void AUK_CharacterBase::ToggleMouse()
{
	if (PC == nullptr)
		return;

	PC->ToggleMouseCursor();
}

void AUK_CharacterBase::Interaction()
{
	if (InteractionComp)
	{
		InteractionComp->TryInteract();
	}
	UE_LOG(LogTemp, Log, TEXT("상호 작용 시도"));
}

void AUK_CharacterBase::Setting()
{
	if (PC == nullptr)
		return;

	PC->Setting_UI();
}

void AUK_CharacterBase::Inventory()
{
}

void AUK_CharacterBase::WeaponCrafting()
{
}

void AUK_CharacterBase::Esc()
{
}

#pragma endregion

#pragma region LockOn

void AUK_CharacterBase::LockON()
{
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

void AUK_CharacterBase::LockONToggle()
{
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

void AUK_CharacterBase::LockONTick()
{
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

void AUK_CharacterBase::AddTarget(const TObjectPtr<AAIMonsterBase> Monster)
{
	LockOnList.AddUnique(Monster);
}

#pragma endregion

#pragma region Gliding

bool AUK_CharacterBase::StartGliding()
{
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

void AUK_CharacterBase::EndGliding()
{
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

bool AUK_CharacterBase::ActorTrace()
{
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

void AUK_CharacterBase::Climb(FHitResult& Hit)
{
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

void AUK_CharacterBase::StartSprintCost()
{
	bIsSprinted = true;
	bInUseStamina = true;
	FGameplayTagContainer TagContainer;
	TagContainer.AddTag(UK_GameplayTags::Input::Sprint);
	GetAbilitySystemComponent()->TryActivateAbilitiesByTag(TagContainer);
}

void AUK_CharacterBase::EndSprintCost()
{
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

void AUK_CharacterBase::ChangeWeaponStat(const FUK_WeaponItemData* WeaponStat)
{
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

void AUK_CharacterBase::EquipWeapon(FGameplayTag NewWeapon)
{
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

void AUK_CharacterBase::SlotWeaponOne()
{
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

void AUK_CharacterBase::SlotWeaponTwo()
{
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

void AUK_CharacterBase::SlotWeaponThree()
{
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

void AUK_CharacterBase::SwapWeapon(int32 Index)
{
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

void AUK_CharacterBase::StopJumpAndFly()
{
	bIsFry = true;
	UCharacterMovementComponent* PlayerMovement = GetCharacterMovement();
	StopJumping();
	PlayerMovement->Velocity = FVector::ZeroVector;
	PlayerMovement->GravityScale = 0.f;

	PlayerMovement->SetJumpAllowed(false);
}

void AUK_CharacterBase::EndComboAttack()
{
	if (bIsFry == false)
		return;
	UCharacterMovementComponent* PlayerMovement = GetCharacterMovement();
	PlayerMovement->GravityScale = DefualtGravity;
	PlayerMovement->SetMovementMode(EMovementMode::MOVE_Walking);
	PlayerMovement->SetJumpAllowed(true);
	bIsFry = false;
}

void AUK_CharacterBase::Dead()
{
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

void AUK_CharacterBase::StartBattle()
{
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

void AUK_CharacterBase::EndBattle()
{
	bInBattle = false;

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

	// 사운드 매니저를 찾아서 전투 상태를 False로 변경
	if (SoundManager)
	{
		SoundManager->SetCombatState(false);
	}
}

void AUK_CharacterBase::HandleLevelUp()
{
	// 1. VFX 재생
	if (LevelUpVFX)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), LevelUpVFX, GetActorLocation());
	}

	// 2. 사운드 재생
	if (LevelUpSound)
	{
		UGameplayStatics::PlaySound2D(GetWorld(), LevelUpSound);
	}
}

#pragma endregion

#pragma region FindMonsterHPBar

void AUK_CharacterBase::UpdateMonsterDetection()
{
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
