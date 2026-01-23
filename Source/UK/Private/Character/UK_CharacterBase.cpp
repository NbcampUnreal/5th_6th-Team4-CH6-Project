// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/UK_CharacterBase.h"
#include "Controller/UK_PlayerController.h"
#include "PlayerState/UK_PlayerState.h"
#include "Actorcomponent/StatusComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "AbilitySystemComponent.h"
#include "Kismet/KismetSystemLibrary.h"

#pragma region Defualt

// Sets default values
AUK_CharacterBase::AUK_CharacterBase() :
	bSprint(false)
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	GetMesh()->SetRelativeLocationAndRotation(
		FVector(0.f, 0.f, -90.f),
		FRotator(0.f, -90.f, 0.f));

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(GetRootComponent());
	SpringArm->TargetArmLength = 300.f;
	SpringArm->SetRelativeLocation(FVector(0.f, 20.f, 40.f));
	SpringArm->bUsePawnControlRotation = true;
	SpringArm->bEnableCameraLag = true; // 카메라가 캐릭터를 뒤늦게 따라옴
	SpringArm->CameraLagSpeed = 5.0f; // 따라오는 속도
	SpringArm->CameraLagMaxDistance = 80.0f; // 카메라와 본래 위치와의 최대 거리 차이

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 480.f, 0.0f);

	StatusComponent = CreateDefaultSubobject<UStatusComponent>(TEXT("StatusComponent"));
}

// Called when the game starts or when spawned
void AUK_CharacterBase::BeginPlay()
{
	Super::BeginPlay();
	
}

void AUK_CharacterBase::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	if (!IsValid(GetAbilitySystemComponent()))
		return;

	GetAbilitySystemComponent()->InitAbilityActorInfo(GetPlayerState(), this);
}

void AUK_CharacterBase::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	if (!IsValid(GetAbilitySystemComponent()))
		return;

	GetAbilitySystemComponent()->InitAbilityActorInfo(GetPlayerState(), this);
	GiveStartupAbilities();
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

// Called to bind functionality to input
void AUK_CharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	if (TObjectPtr<UEnhancedInputComponent> EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (TObjectPtr<AUK_PlayerController> UKPC = Cast<AUK_PlayerController>(GetController()))
		{
			if (UKPC->JumpAction)
			{
				EnhancedInput->BindAction(
					UKPC->JumpAction,
					ETriggerEvent::Started,
					this,
					&ACharacter::Jump);

				EnhancedInput->BindAction(
					UKPC->JumpAction,
					ETriggerEvent::Completed,
					this,
					&ACharacter::StopJumping);
			}

			if (UKPC->LookAction)
			{
				EnhancedInput->BindAction(
					UKPC->LookAction,
					ETriggerEvent::Triggered,
					this,
					&ThisClass::Look);
			}

			if (UKPC->MoveAction)
			{
				EnhancedInput->BindAction(
					UKPC->MoveAction,
					ETriggerEvent::Triggered,
					this,
					&ThisClass::Move);
			}

			if (UKPC->SprintAction)
			{
				EnhancedInput->BindAction(
					UKPC->SprintAction,
					ETriggerEvent::Triggered,
					this,
					&ThisClass::Sprint);
			}

			if (UKPC->AttackAction)
			{
				EnhancedInput->BindAction(
					UKPC->AttackAction,
					ETriggerEvent::Started,
					this,
					&ThisClass::Attack);
			}

			if (UKPC->ZoomIn)
			{
				EnhancedInput->BindAction(
					UKPC->ZoomIn,
					ETriggerEvent::Triggered,
					this,
					&ThisClass::ZoomIn);
			}

			if (UKPC->ZoomOut)
			{
				EnhancedInput->BindAction(
					UKPC->ZoomOut,
					ETriggerEvent::Triggered,
					this,
					&ThisClass::ZoomOut);
			}
		}
	}
}

void AUK_CharacterBase::Sprint()
{

	if (!bSprint)
	{
		GetCharacterMovement()->MaxWalkSpeed = 1200.f;
		bSprint = true;
	}
	else
	{
		GetCharacterMovement()->MaxWalkSpeed = 600.f;
		bSprint = false;
	}
}

void AUK_CharacterBase::Move(const FInputActionValue& Value)
{
	if (!Controller)
		return;
	const FVector2D MoveInput = Value.Get<FVector2D>();

	if (!FMath::IsNearlyZero(MoveInput.X))
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, MoveInput.X);
	}


	if (!FMath::IsNearlyZero(MoveInput.Y))
	{

		const FRotator Rotation = Controller->GetControlRotation();

		const FRotator YawRotation(0, Rotation.Yaw, 0);
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(Direction, MoveInput.Y);
	}
}
void AUK_CharacterBase::Look(const FInputActionValue& Value)
{
	const FVector2D LookInput = Value.Get<FVector2D>();

	AddControllerYawInput(LookInput.X);
	AddControllerPitchInput(LookInput.Y);
}

void AUK_CharacterBase::Attack()
{
	UE_LOG(LogTemp, Warning, TEXT("Attack"));
}

void AUK_CharacterBase::ZoomIn()
{
	const float DeltaTime = GetWorld()->GetDeltaSeconds();

	const float Target = 70.f;
	SpringArm->TargetArmLength = FMath::FInterpTo(
		SpringArm->TargetArmLength,
		Target,
		DeltaTime,
		12.f
	);
}

void AUK_CharacterBase::ZoomOut()
{
	const float DeltaTime = GetWorld()->GetDeltaSeconds();

	const float Target = 300.f;
	SpringArm->TargetArmLength = FMath::FInterpTo(
		SpringArm->TargetArmLength,
		Target,
		DeltaTime,
		12.f
	);
}
#pragma endregion
