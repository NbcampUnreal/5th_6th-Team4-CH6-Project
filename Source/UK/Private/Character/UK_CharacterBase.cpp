// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/UK_CharacterBase.h"
#include "Character/UK_PlayerController.h"
#include "Character/UK_PlayerState.h"
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
#include "Kismet/KismetMathLibrary.h"
#include "Engine/OverlapResult.h"
#include "Sound/SoundAttenuation.h"
#include "Systems/UK_GameInstance.h"
#include "DataAsset/Data/UK_WeaponItemData.h"


#pragma region Defualt


// 무현님 대머리 ㅋㅋ
// Sets default values
AUK_CharacterBase::AUK_CharacterBase() :
	bIsLock(false),
	bIsCrouched(false),
	SprintSpeed(800.f),
	GlideFallSpeed(200.f),
	NowWeapon(nullptr)
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = false;

	GetMesh()->SetRelativeLocationAndRotation(
		FVector(0.f, 0.f, -90.f),
		FRotator(0.f, -90.f, 0.f));
	GetMesh()->SetCollisionProfileName(TEXT("UK_Charactor"));
#pragma region SpringArm

	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArmComp->SetupAttachment(GetRootComponent());
	SpringArmComp->TargetArmLength = 300.f;
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

//// Called every frame
//void AUK_CharacterBase::Tick(float DeltaTime)
//{
//	Super::Tick(DeltaTime);
//
//}

void AUK_CharacterBase::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

}

void AUK_CharacterBase::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);
	if (MovementMode == ECustomMovementMode::CMOVE_Glide)
	{
		EndGliding();
	}
	if (OnFloor.IsBound() == true)
	{
		OnFloor.Execute();
	}

	FGameplayEventData EventData;
	EventData.EventTag = UK_GameplayTags::Action::DropAttack;
	GetAbilitySystemComponent()->HandleGameplayEvent(EventData.EventTag, &EventData);
}

void AUK_CharacterBase::ChangedAttribute(ECharacterAttribute NewAttribute)
{
	Attribute = NewAttribute;
	OnChangedAttribute.Broadcast(NewAttribute);
}

// Called when the game starts or when spawned
void AUK_CharacterBase::BeginPlay()
{
	Super::BeginPlay();

	PC = Cast<AUK_PlayerController>(GetController());

	GetWorldTimerManager().SetTimer(
		DetectTimer,
		this,
		&AUK_CharacterBase::UpdateMonsterDetection,
		0.3f,
		true
	);
	DefualtGravity = GetCharacterMovement()->GravityScale;
	DefualtAirControl = GetCharacterMovement()->AirControl;
	if (!IsValid(GetAbilitySystemComponent()))
		return;

	GetAbilitySystemComponent()->InitAbilityActorInfo(GetPlayerState(), this);
	GiveStartupAbilities();
	AUK_PlayerState* PS = Cast<AUK_PlayerState>(GetPlayerState());
	if (IsValid(PS))
	{
		PS->InitializeAttributes();
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
	UKInputComp->BindAction(InputMappingConfig->FindNativeInputActionByTag(UK_GameplayTags::Input::Sprint),
	                        ETriggerEvent::Started, this, &ThisClass::Sprint);
	UKInputComp->BindAction(InputMappingConfig->FindNativeInputActionByTag(UK_GameplayTags::Input::ZoomIn),
	                        ETriggerEvent::Triggered, this, &ThisClass::ZoomIn);
	UKInputComp->BindAction(InputMappingConfig->FindNativeInputActionByTag(UK_GameplayTags::Input::ZoomOut),
	                        ETriggerEvent::Triggered, this, &ThisClass::ZoomOut);
	UKInputComp->BindAction(InputMappingConfig->FindNativeInputActionByTag(UK_GameplayTags::Action::LightAttack),
	                        ETriggerEvent::Started, this, &ThisClass::LightAttack);
	UKInputComp->BindAction(InputMappingConfig->FindNativeInputActionByTag(UK_GameplayTags::Action::HeavyAttack),
	                        ETriggerEvent::Started, this, &ThisClass::HeavyAttack);
	UKInputComp->BindAction(InputMappingConfig->FindNativeInputActionByTag(UK_GameplayTags::Input::Crouch),
	                        ETriggerEvent::Started, this, &ThisClass::CrouchInput);
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
	                        ETriggerEvent::Started, this, &ThisClass::NomalSkill);
	UKInputComp->BindAction(InputMappingConfig->FindNativeInputActionByTag(UK_GameplayTags::Input::UltimateSkill),
	                        ETriggerEvent::Started, this, &ThisClass::UltimateSkill);
	UKInputComp->BindAction(InputMappingConfig->FindNativeInputActionByTag(UK_GameplayTags::Action::Parry),
	                        ETriggerEvent::Started, this, &ThisClass::Parry);
	UKInputComp->BindAction(InputMappingConfig->FindNativeInputActionByTag(UK_GameplayTags::Input::Dash),
	                        ETriggerEvent::Started, this, &ThisClass::Dash);
}

float AUK_CharacterBase::GetFloorDistance()
{
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
	const FVector2D MovementVector = InputActionValue.Get<FVector2D>();
	const FRotator MovementRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);

	FVector ForwardDirection;
	FVector RightDirection;
	switch (MovementMode)
	{
	case ECustomMovementMode::CMOVE_Glide:

		ForwardDirection = Controller->GetControlRotation().Vector();
		RightDirection = FRotationMatrix(MovementRotation).GetUnitAxis(EAxis::Y);
		break;
	default:
		ForwardDirection = FRotationMatrix(MovementRotation).GetUnitAxis(EAxis::X);
		RightDirection = FRotationMatrix(MovementRotation).GetUnitAxis(EAxis::Y);
		break;
	}
	if (FMath::IsNearlyZero(MovementVector.X) == false)
	{
		AddMovementInput(ForwardDirection, MovementVector.X);
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

void AUK_CharacterBase::Sprint()
{
	if (bIsSprinted == false)
	{
		GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
		bIsSprinted = true;
	}
	else if (bIsSprinted == true)
	{
		GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
		bIsSprinted = false;
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

	constexpr float Target = 300.f;
	SpringArmComp->TargetArmLength = FMath::FInterpTo(
		SpringArmComp->TargetArmLength,
		Target,
		DeltaTime,
		12.f
	);
}

void AUK_CharacterBase::LightAttack()
{
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

void AUK_CharacterBase::NomalSkill()
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

void AUK_CharacterBase::CrouchInput()
{
	if (GetCharacterMovement()->IsFalling() == true)
		return;

	if (bIsCrouched == true)
	{
		UnCrouch();
		bIsCrouched = false;
	}
	else if (bIsCrouched == false)
	{
		Crouch();
		bIsCrouched = true;
	}
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

void AUK_CharacterBase::Dash()
{
	InputType = EInputMode::Dash;
	FGameplayTagContainer Container;
	Container.AddTag(UK_GameplayTags::Input::Dash);
	GetAbilitySystemComponent()->TryActivateAbilitiesByTag(Container);
}
#pragma endregion

#pragma region LockOn

void AUK_CharacterBase::LockON()
{
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
						0.01f,
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
	if (LockOnList.IsEmpty() == false)
	{
		if (const int32 Size = LockOnList.Num(); Size <= index)
		{
			index = 0;
		}
		const AAIMonsterBase* Monster = LockOnList[index];
		if (IsValid(Monster) == false)
		{
			LockOnList.RemoveAtSwap(index);
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
			LockOnList.RemoveAtSwap(index);
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
	if (MovementMode == ECustomMovementMode::CMOVE_Glide)
	{
		return false;
	}
	if ( GetFloorDistance() < 220.f)
	{
		return false;
	}
	GetCharacterMovement()->StopMovementImmediately();
	FVector Vel = GetCharacterMovement()->Velocity;
	Vel.Z = -GlideFallSpeed;
	GetCharacterMovement()->GravityScale = 0.f;
	GetCharacterMovement()->AirControl = 0.8;
	GetCharacterMovement()->Velocity = Vel;
	MovementMode = ECustomMovementMode::CMOVE_Glide;
	return true;
}

void AUK_CharacterBase::EndGliding()
{
	MovementMode = ECustomMovementMode::CMOVE_None;

	GetCharacterMovement()->GravityScale = DefualtGravity;
	GetCharacterMovement()->AirControl = DefualtAirControl;
}

#pragma endregion

#pragma endregion

#pragma region Weapon

void AUK_CharacterBase::EquipWeapon(FGameplayTag NewWeapon)
{
	UUK_StatusAnimData* Weapon = WeaponList->FindAnimsDataAssetByTag(NewWeapon);
	NowWeapon = Weapon;
	if (IsValid(Weapon->GetRightHandWeapon()))
	{
		RightHandWeaponComponent->SetSkeletalMesh(Weapon->GetRightHandWeapon());

		RightHandWeaponComponent->SetRelativeLocation(Weapon->GetRightLocationOffset());
		RightHandWeaponComponent->SetRelativeRotation(Weapon->GetRightRotationOffset());
	}
	else
	{
		RightHandWeaponComponent->SetSkeletalMesh(nullptr);
	}
	if (IsValid(Weapon->GetLeftHandWeapon()))
	{
		LeftHandWeaponComponent->SetSkeletalMesh(Weapon->GetLeftHandWeapon());

		LeftHandWeaponComponent->SetRelativeLocation(Weapon->GetLeftLocationOffset());
		LeftHandWeaponComponent->SetRelativeRotation(Weapon->GetLeftRotationOffset());
	}
	else
	{
		LeftHandWeaponComponent->SetSkeletalMesh(nullptr);
	}
}

void AUK_CharacterBase::SlotWeaponOne()
{
	FGameplayTagContainer Container;
	Container.AddTag(UK_GameplayTags::Action::Swap1);
	GetAbilitySystemComponent()->TryActivateAbilitiesByTag(Container);
}

void AUK_CharacterBase::SlotWeaponTwo()
{
	FGameplayTagContainer Container;
	Container.AddTag(UK_GameplayTags::Action::Swap2);
	GetAbilitySystemComponent()->TryActivateAbilitiesByTag(Container);
}

void AUK_CharacterBase::SlotWeaponThree()
{
	FGameplayTagContainer Container;
	Container.AddTag(UK_GameplayTags::Action::Swap3);
	GetAbilitySystemComponent()->TryActivateAbilitiesByTag(Container);
}

void AUK_CharacterBase::SwapWeapon(int32 Index)
{
	if (IsValid(WeaponDataTable) == false)
	{
		UE_LOG(LogTemp, Display, TEXT("ItmeDataTable is Nullptr"));
		return;
	}
	FInventorySlot* WeaponSlot = InventoryComponent->FindWeaponSlotbyIndex(Index);
	if (WeaponSlot->isEmpty())
	{
		return;
	}
	const FUK_WeaponItemData* ItemData = WeaponDataTable->FindRow<FUK_WeaponItemData>(
		WeaponSlot->ItemID, TEXT("AUK_CharacterBase::SwapWeapon"));
	if (ItemData == nullptr)
	{
		return;
	}
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
	UE_LOG(LogTemp, Display, TEXT("Is Player Dead"));
	GetCharacterMovement()->DisableMovement();
	GetController()->SetIgnoreMoveInput(true);
	GetController()->SetIgnoreLookInput(true);
	GetAbilitySystemComponent()->AddLooseGameplayTag(UK_GameplayTags::Status::Dead);
	OnDead.Broadcast();
}

void AUK_CharacterBase::UpdateMonsterDetection()
{
	if (!IsLocallyControlled())
		return;
	TArray<FOverlapResult> Results;
	FCollisionShape Sphere = FCollisionShape::MakeSphere(DetectRadius);
	GetWorld()->OverlapMultiByObjectType(Results, GetActorLocation(), FQuat::Identity,
	                                     FCollisionObjectQueryParams(ECC_Pawn), Sphere);
	if ( bDrawDetectRadius )
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
