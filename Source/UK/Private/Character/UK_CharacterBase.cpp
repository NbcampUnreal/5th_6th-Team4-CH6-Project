// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/UK_CharacterBase.h"
#include "Character/UK_PlayerController.h"
#include "Character/UK_PlayerState.h"
#include "Character/Weapon/UK_WeaponBase.h"
#include "AIMonster/AIMonsterBase.h"
#include "AIMonster/Component/AI_MonsterStatComponent.h"
#include "Tags/UK_GameplayTags.h"
#include "Animation/UK_AnimInstance.h"
#include "ActorComponent/StatusComponent.h"
#include "ActorComponent/UK_CombatAnimationComponent.h"
#include "ActorComponent/UK_InventoryComponent.h"
#include "ActorComponent/UK_InputComponent.h"
#include "NPC/Component/UK_InteractionComponent.h"
#include "NPC/Component/UK_QuestComponent.h"
#include "DataAsset/UK_WeaponData.h"
#include "DataAsset/UK_StatusAnimData.h"
#include "DataAsset/UK_InputConfig.h"
#include "DataAsset/Data/UK_ItemData.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "AbilitySystemComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Engine/OverlapResult.h"
#include "Blueprint/UserWidget.h"

#pragma region Defualt



// Sets default values
AUK_CharacterBase::AUK_CharacterBase() :
	NowWeapon(nullptr),
	bIsLock(false),
	bIsCrouched(false)
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

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

	RightHandWeaponComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("RightHandWeaponComponent"));
	RightHandWeaponComponent->SetupAttachment(GetMesh(), TEXT("Weapon_rSocket"));
	RightHandWeaponComponent->SetLeaderPoseComponent(GetMesh());

	LeftHandWeaponComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("LeftHandWeaponComponent"));
	LeftHandWeaponComponent->SetupAttachment(GetMesh(), TEXT("Weapon_lSocket"));
	LeftHandWeaponComponent->SetLeaderPoseComponent(GetMesh());

	StatusComponent = CreateDefaultSubobject<UStatusComponent>(TEXT("StatusComponent"));
	InventoryComponent = CreateDefaultSubobject<UUK_InventoryComponent>(TEXT("InventoryComponent"));
	InteractionComp = CreateDefaultSubobject<UUK_InteractionComponent>(TEXT("InteractionComponent"));
	QuestComp = CreateDefaultSubobject<UUK_QuestComponent>(TEXT("QuestComponent"));
}

void AUK_CharacterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AUK_CharacterBase, StatusComponent, COND_None);
	DOREPLIFETIME_CONDITION(AUK_CharacterBase, CurrentWeaponTag, COND_None);
	DOREPLIFETIME_CONDITION(AUK_CharacterBase, bIsInInput, COND_None);

}

void AUK_CharacterBase::OnRep_RightHandWeapon()
{
}

// Called when the game starts or when spawned
void AUK_CharacterBase::BeginPlay()
{
	Super::BeginPlay();

	StatusComponent->OnDeadDelegate.AddDynamic(this, &AUK_CharacterBase::Dead);
	DefaultGravityValue = GetCharacterMovement()->GravityScale;

	PC = Cast<AUK_PlayerController>(GetController());

	GetWorldTimerManager().SetTimer(
		DetectTimer,
		this,
		&AUK_CharacterBase::UpdateMonsterDetection,
		0.3f,
		true
	);
}

void AUK_CharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* UKInputComp = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if ( IsValid(UKInputComp) == false )
	{
		return;
	}
	UKInputComp->BindAction(InputMappingConfig->FindNativeInputActionByTag(UK_GameplayTags::Input::Move), ETriggerEvent::Triggered, this, &ThisClass::Move);
	UKInputComp->BindAction(InputMappingConfig->FindNativeInputActionByTag(UK_GameplayTags::Input::Look), ETriggerEvent::Triggered, this, &AUK_CharacterBase::Look);
	UKInputComp->BindAction(InputMappingConfig->FindNativeInputActionByTag(UK_GameplayTags::Input::Jump), ETriggerEvent::Started, this, &ThisClass::Jump);
	UKInputComp->BindAction(InputMappingConfig->FindNativeInputActionByTag(UK_GameplayTags::Input::Jump), ETriggerEvent::Canceled, this, &ThisClass::StopJumping);
	UKInputComp->BindAction(InputMappingConfig->FindNativeInputActionByTag(UK_GameplayTags::Input::Sprint), ETriggerEvent::Started, this, &ThisClass::Sprint);
	UKInputComp->BindAction(InputMappingConfig->FindNativeInputActionByTag(UK_GameplayTags::Input::ZoomIn), ETriggerEvent::Triggered, this, &ThisClass::ZoomIn);
	UKInputComp->BindAction(InputMappingConfig->FindNativeInputActionByTag(UK_GameplayTags::Input::ZoomOut), ETriggerEvent::Triggered, this, &ThisClass::ZoomOut);
	UKInputComp->BindAction(InputMappingConfig->FindNativeInputActionByTag(UK_GameplayTags::Action::LightAttack), ETriggerEvent::Started, this, &ThisClass::LightAttack);
	UKInputComp->BindAction(InputMappingConfig->FindNativeInputActionByTag(UK_GameplayTags::Action::HeavyAttack), ETriggerEvent::Started, this, &ThisClass::HeavyAttack);
	UKInputComp->BindAction(InputMappingConfig->FindNativeInputActionByTag(UK_GameplayTags::Input::Crouch), ETriggerEvent::Started, this, &ThisClass::CrouchInput);
	UKInputComp->BindAction(InputMappingConfig->FindNativeInputActionByTag(UK_GameplayTags::Input::ToggleMouse), ETriggerEvent::Started, this, &ThisClass::ToggleMouse);
	UKInputComp->BindAction(InputMappingConfig->FindNativeInputActionByTag(UK_GameplayTags::Input::Interaction), ETriggerEvent::Started, this, &ThisClass::Interaction);
	UKInputComp->BindAction(InputMappingConfig->FindNativeInputActionByTag(UK_GameplayTags::Input::Setting), ETriggerEvent::Started, this, &ThisClass::Setting);
}

void AUK_CharacterBase::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	if ( !IsValid(GetAbilitySystemComponent()) )
		return;

	GetAbilitySystemComponent()->InitAbilityActorInfo(GetPlayerState(), this);
}

void AUK_CharacterBase::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	if ( !IsValid(GetAbilitySystemComponent()) )
		return;

	GetAbilitySystemComponent()->InitAbilityActorInfo(GetPlayerState(), this);
	GiveStartupAbilities();
}

void AUK_CharacterBase::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);
	if ( OnFloor.IsBound() == true )
	{
		OnFloor.Execute();

	}

	FGameplayEventData EventData;
	EventData.EventTag = FGameplayTag::RequestGameplayTag("Action.DropAttack");

	GetAbilitySystemComponent()->HandleGameplayEvent(EventData.EventTag, &EventData);
}

//// Called every frame
//void AUK_CharacterBase::Tick(float DeltaTime)
//{
//	Super::Tick(DeltaTime);
//
//}
#pragma endregion

#pragma region GAS

UAbilitySystemComponent* AUK_CharacterBase::GetAbilitySystemComponent() const
{
	AUK_PlayerState* UKPS = Cast<AUK_PlayerState>(GetPlayerState());
	if ( !IsValid(UKPS) )
		return nullptr;

	return UKPS->GetAbilitySystemComponent();
}

void AUK_CharacterBase::GiveStartupAbilities()
{
	if ( !IsValid(GetAbilitySystemComponent()) )
		return;

	for ( const TSubclassOf<UGameplayAbility>& Ability : StartupAbilities )
	{
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(Ability);
		GetAbilitySystemComponent()->GiveAbility(AbilitySpec);
	}
}
#pragma endregion

#pragma region Input

void AUK_CharacterBase::Move(const FInputActionValue& InputActionValue)
{
	const FVector2D MovementVector = InputActionValue.Get<FVector2D>();
	const FRotator MovementRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);

	if ( FMath::IsNearlyZero(MovementVector.X) == false )
	{
		const FVector ForwardDirection = FRotationMatrix(MovementRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(ForwardDirection, MovementVector.X);
	}

	if ( FMath::IsNearlyZero(MovementVector.Y) == false )
	{
		const FVector RightDirection = FRotationMatrix(MovementRotation).GetUnitAxis(EAxis::Y);
		AddMovementInput(RightDirection, MovementVector.Y);
	}
}

void AUK_CharacterBase::Look(const FInputActionValue& InputActionValue)
{
	const FVector2D LookAxisVector = InputActionValue.Get<FVector2D>();

	if ( FMath::IsNearlyZero(LookAxisVector.X) == false )
	{
		AddControllerYawInput(LookAxisVector.X);
	}
	if ( FMath::IsNearlyZero(LookAxisVector.Y) == false )
	{
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void AUK_CharacterBase::Sprint()
{
	if ( StatusComponent->IsDead() )
		return;

	if ( bIsSprinted == false )
	{
		GetCharacterMovement()->MaxWalkSpeed = 800.f;
		bIsSprinted = true;
	}
	else if ( bIsSprinted == true )
	{
		GetCharacterMovement()->MaxWalkSpeed = 500.f;
		bIsSprinted = false;
	}
}

void AUK_CharacterBase::LightAttack()
{
	if ( StatusComponent->IsDead() )
	{
		return;
	}
	bIsInInput = true;
	FGameplayTagContainer Container;
	if ( GetCharacterMovement()->IsFalling() == true )
	{
		Container.AddTag(UK_GameplayTags::Action::AirAttack);
		GetAbilitySystemComponent()->TryActivateAbilitiesByTag(Container);
	}
	else if ( GetCharacterMovement()->IsFalling() == false )
	{
		Container.AddTag(UK_GameplayTags::Action::LightAttack);
		GetAbilitySystemComponent()->TryActivateAbilitiesByTag(Container);
	}
}
void AUK_CharacterBase::HeavyAttack()
{
	if ( StatusComponent->IsDead() )
	{
		return;
	}
	bIsInInput = true;
	FGameplayTagContainer Container;
	Container.AddTag(UK_GameplayTags::Action::HeavyAttack);
	GetAbilitySystemComponent()->TryActivateAbilitiesByTag(Container);

}

void AUK_CharacterBase::CrouchInput()
{
	if ( StatusComponent->IsDead() )
		return;
	if ( GetCharacterMovement()->IsFalling() == true )
		return;

	if ( bIsCrouched == true )
	{
		UnCrouch();
		bIsCrouched = false;
	}
	else if ( bIsCrouched == false )
	{
		Crouch();
		bIsCrouched = true;
	}
}

void AUK_CharacterBase::ToggleMouse()
{
	if ( StatusComponent->IsDead() )
		return;

	if ( PC == nullptr )
		return;

	PC->ToggleMouseCursor();
}

void AUK_CharacterBase::Interaction()
{
	if ( InteractionComp )
	{
		InteractionComp->TryInteract();
	}
	UE_LOG(LogTemp, Log, TEXT("상호 작용 시도"));
}

void AUK_CharacterBase::Setting()
{
	if ( StatusComponent->IsDead() )
		return;

	if ( PC == nullptr )
		return;

	PC->Setting_UI();
}

void AUK_CharacterBase::ZoomIn()
{
	if ( StatusComponent->IsDead() )
	{
		return;
	}

	if ( !IsValid(SpringArmComp) )
	{
		return;
	}
	const float DeltaTime = GetWorld()->GetDeltaSeconds();

	const float Target = 70.f;
	SpringArmComp->TargetArmLength = FMath::FInterpTo(
		SpringArmComp->TargetArmLength,
		Target,
		DeltaTime,
		12.f
	);
}

void AUK_CharacterBase::ZoomOut()
{
	if ( StatusComponent->IsDead() )
	{
		return;
	}
	if ( !IsValid(SpringArmComp) )
	{
		return;
	}
	const float DeltaTime = GetWorld()->GetDeltaSeconds();

	const float Target = 300.f;
	SpringArmComp->TargetArmLength = FMath::FInterpTo(
		SpringArmComp->TargetArmLength,
		Target,
		DeltaTime,
		12.f
	);
}

void AUK_CharacterBase::LockON()
{
	if ( bIsLock == false )
	{
		//	AUK_PlayerController* UKPC = Cast<AUK_PlayerController>(GetController());
		//	if ( IsValid(UKPC) == false )
		//		return;
		//	FVector Start;
		//	FRotator CameraRot;
		//	const float CapsuleRadius = 50.f;
		//	// 카메라부터 카메라가 보는 방향으로 트레이스 실시
		//	UKPC->GetPlayerViewPoint(Start, CameraRot);
		//	FVector ForwardVector = CameraRot.Vector();/*카메라의 방향성*/

		//	float TraceDistance = 1000.f;
		//	FVector End = Start + ( ForwardVector * TraceDistance );

		//	FCollisionQueryParams Params;
		//	Params.AddIgnoredActor(this);
		//	FCollisionShape CollisionShape = FCollisionShape::MakeSphere(CapsuleRadius);

		//	bool bHit = GetWorld()->SweepMultiByChannel(
		//		LockOnResult,
		//		Start,
		//		End,
		//		FQuat::Identity,
		//		ECC_LockOn,/*추후에 카메라 전용 트레이스 채널로 변경 요망*/
		//		CollisionShape,
		//		Params
		//	);
		//	FColor DrawColor = bHit ? FColor::Green : FColor::Red;

		//	FQuat CapsuleRot = FRotationMatrix::MakeFromZ(Start - End).ToQuat();
		//	DrawDebugCapsule(
		//		GetWorld(),
		//		( Start + End ) / 2,
		//		( End - Start ).Size(),
		//		CapsuleRadius,
		//		CapsuleRot,
		//		DrawColor,
		//		false,
		//		1.f
		//	);
		//	if ( bHit )
		//	{
		bIsLock = true;
		bUseControllerRotationYaw = true;
		GetCharacterMovement()->bOrientRotationToMovement = false;
		GetWorld()->GetTimerManager().SetTimer(
			LockOnTimer,
			this,
			&AUK_CharacterBase::LockONTick,
			0.01f,
			true
		);

		//}
	}
	//else
	//{
	//	bIsLock = false;
	//	GetWorld()->GetTimerManager().ClearTimer(LockOnTimer);
	//	LockOnList.Reset();
	//	LockOnTimer.Invalidate();
	//}
}
void AUK_CharacterBase::LockONTick()
{
	if ( LockOnList.IsEmpty() == false )
	{
		int32 size = LockOnList.Num();
		if ( size <= index )
		{
			index = 0;
		}
		AAIMonsterBase* Monster = LockOnList[ index ];
		if ( IsValid(Monster) == false )
		{
			LockOnList.RemoveAtSwap(index);
			return;
		}

		if ( Monster->IsDead() == false )
		{
			float Distance = FVector::Dist(GetActorLocation(), Monster->GetActorLocation());
			if ( Distance > MaxLockDistance )
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
			FVector Start = GetActorLocation();
			FVector End = Monster->GetActorLocation();
			FRotator Target = UKismetMathLibrary::FindLookAtRotation(Start, End);
			FRotator NowRot = GetController()->GetControlRotation();
			FRotator NewRot = FMath::RInterpTo(
				NowRot,
				Target,
				DeltaTime,
				12.f
			);

			GetController()->SetControlRotation(NewRot);

		}
		else if ( Monster->IsDead() == true )
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

#pragma region Weapon

void AUK_CharacterBase::EquipWeapon(FGameplayTag NewWeapon)
{
	CurrentWeaponTag = NewWeapon;
	UUK_StatusAnimData* Weapon = WeaponList->FindAnimsDataAssetByTag(NewWeapon);
	NowWeapon = Weapon;
	if ( IsValid(Weapon->GetRightHandWeapon()) )
	{
		RightHandWeaponComponent->SetSkeletalMesh(Weapon->GetRightHandWeapon());         // todo : 이후에 서버에서 변경하도록 수정해야함 임시로 클라에서만 변경하고 있음

		RightHandWeaponComponent->SetRelativeLocation(Weapon->GetRightLocationOffset());
		RightHandWeaponComponent->SetRelativeRotation(Weapon->GetRightRotationOffset());
	}
	else
	{
		RightHandWeaponComponent->SetSkeletalMesh(nullptr);
	}
	if ( IsValid(Weapon->GetLeftHandWeapon()) )
	{
		LeftHandWeaponComponent->SetSkeletalMesh(Weapon->GetLeftHandWeapon());             // todo : 이후에 서버에서 변경하도록 수정해야함 임시로 클라에서만 변경하고 있음

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
	FInventorySlot* WeaponSlot = InventoryComponent->FindWeaponSlotbyIndex(Index);
	if ( WeaponSlot->isEmpty() )
	{
		return;
	}
	const FUK_ItemData* ItemData = ItmeDataTable->FindRow<FUK_ItemData>(WeaponSlot->ItemID, TEXT("AUK_CharacterBase::SwapWeapon"));
	if ( ItemData == nullptr )
	{
		return;
	}
	EquipWeapon(ItemData->ItemTag);
}
void AUK_CharacterBase::OnRep_CurrentWeaponTag()
{
	UUK_StatusAnimData* Weapon = WeaponList->FindAnimsDataAssetByTag(CurrentWeaponTag);
	NowWeapon = Weapon;
	if ( IsValid(Weapon->GetRightHandWeapon()) )
	{
		RightHandWeaponComponent->SetSkeletalMesh(Weapon->GetRightHandWeapon());         // todo : 이후에 서버에서 변경하도록 수정해야함 임시로 클라에서만 변경하고 있음

		RightHandWeaponComponent->SetRelativeLocation(Weapon->GetRightLocationOffset());
		RightHandWeaponComponent->SetRelativeRotation(Weapon->GetRightRotationOffset());
	}
	else
	{
		RightHandWeaponComponent->SetSkeletalMesh(nullptr);
	}
	if ( IsValid(Weapon->GetLeftHandWeapon()) )
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
void AUK_CharacterBase::OnRep_NowWeapon()
{
}
#pragma endregion

#pragma region Battle

void AUK_CharacterBase::StopJumpAndFly()
{
	bIsfry = true;
	UCharacterMovementComponent* PlayerMovement = GetCharacterMovement();

	PlayerMovement->GravityScale = 0.f;
	PlayerMovement->Velocity = FVector::ZeroVector;
	StopJumping();

	PlayerMovement->SetJumpAllowed(false);
}
void AUK_CharacterBase::EndComboAttack()
{
	;
	if ( bIsfry == false )
		return;
	UCharacterMovementComponent* PlayerMovement = GetCharacterMovement();
	GetCharacterMovement()->GravityScale = DefaultGravityValue;
	PlayerMovement->SetMovementMode(EMovementMode::MOVE_Walking);
	PlayerMovement->SetJumpAllowed(true);
	bIsfry = false;
}

void AUK_CharacterBase::ReceiveDamage(float Damage)
{
	if ( !HasAuthority() ) return;

	if ( IsValid(StatusComponent) )
	{
		StatusComponent->TakeDamage(Damage);
	}
}

float AUK_CharacterBase::ApplyDamage()
{
	if ( IsValid(StatusComponent) )
	{
		return StatusComponent->ApplyDamage();
	}
	return 0.f;
}
void AUK_CharacterBase::Dead()
{
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None);
}
void AUK_CharacterBase::OnRep_InInput()
{

}
void AUK_CharacterBase::UpdateMonsterDetection() {
	if ( !IsLocallyControlled() ) 
		return; 
	TArray<FOverlapResult> Results; 
	FCollisionShape Sphere = FCollisionShape::MakeSphere(DetectRadius); 
	GetWorld()->OverlapMultiByObjectType(Results, GetActorLocation(), FQuat::Identity, FCollisionObjectQueryParams(ECC_Pawn), Sphere); 
	DrawDebugSphere(GetWorld(), GetActorLocation(), DetectRadius, 32, FColor::Green, false, 0.31f); TSet<AAIMonsterBase*> NewSet; 
	// overlap이 되는 것들의 data result 결과들
	for ( const FOverlapResult& Result : Results ) 
	{ 
		AActor* OverlappedActor = Result.OverlapObjectHandle.FetchActor(); 
		if ( AAIMonsterBase* Monster = Cast<AAIMonsterBase>(OverlappedActor) ) 
		{ 
			NewSet.Add(Monster); 
			if ( !NearbyMonsters.Contains(Monster) ) 
			{
				Monster->ShowHPBar(); 
			} 
		} 
	} 
	// 범위가 벗어났는지 확인 
	for ( AAIMonsterBase* OldMonster : NearbyMonsters ) 
	{ 
		if ( IsValid(OldMonster) && !NewSet.Contains(OldMonster) ) 
		{ 
			OldMonster->HideHPBar(); 
		} 
	} 
	NearbyMonsters = NewSet; 
}
#pragma endregion
void AUK_CharacterBase::OnRep_fry()
{
}
#pragma endregion

void AUK_CharacterBase::Client_ShowInteractUI_Implementation()
{
	if ( InteractWidget ) return;

	if ( !InteractWidgetClass ) return;

	InteractWidget =
		CreateWidget<UUserWidget>(
			GetWorld(),
			InteractWidgetClass
		);

	if ( InteractWidget )
	{
		InteractWidget->AddToViewport();
	}
}

void AUK_CharacterBase::Client_HideInteractUI_Implementation()
{
	if ( !InteractWidget ) return;

	InteractWidget->RemoveFromParent();
	InteractWidget = nullptr;
}
