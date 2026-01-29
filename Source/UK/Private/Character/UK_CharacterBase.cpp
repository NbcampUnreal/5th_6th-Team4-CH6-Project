// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/UK_CharacterBase.h"
#include "Character/UK_PlayerController.h"
#include "Character/UK_PlayerState.h"
#include "Character/Weapon/UK_WeaponBase.h"
#include "Tags/UK_GameplayTags.h"
#include "Animation/UK_AnimInstance.h"
#include "ActorComponent/StatusComponent.h"
#include "ActorComponent/UK_InputComponent.h"
#include "ActorComponent/UK_CombatAnimationComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "AbilitySystemComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"

#pragma region Defualt

int32 AUK_CharacterBase::ShowAttackDebug = 0;

FAutoConsoleVariableRef CVarShowAttackDebug(
	TEXT("UK.ShowAttackDebug"),
	AUK_CharacterBase::ShowAttackDebug,
	TEXT(""),
	ECVF_Cheat
);

// Sets default values
AUK_CharacterBase::AUK_CharacterBase() :
	bSprint(false),
	AttackRange(50.f),
	AttackRadius(20.f),
	WeaponIndex(0)
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	GetMesh()->SetRelativeLocationAndRotation(
		FVector(0.f, 0.f, -90.f),
		FRotator(0.f, -90.f, 0.f));
	GetMesh()->SetCollisionProfileName(TEXT("UK_Charactor"));
#pragma region SpringArm
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(GetRootComponent());
	SpringArm->TargetArmLength = 300.f;
	SpringArm->SetRelativeLocation(FVector(0.f, 20.f, 40.f));
	SpringArm->bUsePawnControlRotation = true;
	SpringArm->bEnableCameraLag = true; // 카메라가 캐릭터를 뒤늦게 따라옴
	SpringArm->CameraLagSpeed = 5.0f; // 따라오는 속도
	SpringArm->CameraLagMaxDistance = 80.0f; // 카메라와 본래 위치와의 최대 거리 차이
#pragma endregion

#pragma region Camera
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 480.f, 0.0f);
#pragma endregion

	StatusComponent = CreateDefaultSubobject<UStatusComponent>(TEXT("StatusComponent"));
	AnimationComponent = CreateDefaultSubobject<UUK_CombatAnimationComponent>(TEXT("AnimComponent"));
}

void AUK_CharacterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AUK_CharacterBase, StatusComponent, COND_None);
	DOREPLIFETIME_CONDITION(AUK_CharacterBase, Weapons, COND_None);
	DOREPLIFETIME_CONDITION(AUK_CharacterBase, Weapon, COND_None);
}

// Called when the game starts or when spawned
void AUK_CharacterBase::BeginPlay()
{
	Super::BeginPlay();

	if ( HasAuthority() )
	{
		for ( const TSubclassOf<AUK_WeaponBase>& WeaponClass : DefaultWeapons )
		{
			if ( !WeaponClass ) continue;
			FActorSpawnParameters Params;
			Params.Owner = this;
			AUK_WeaponBase* SpawnedWeapon = GetWorld()->SpawnActor<AUK_WeaponBase>(WeaponClass, Params); //서버와 클라이언트에 무기 스폰을 해야하기 때문에 반복문을 이용해준다.
			const int32 Index = Weapons.Add(SpawnedWeapon);
			if ( Index == WeaponIndex )
			{
				Weapon = SpawnedWeapon;
				OnRep_CurrentWeapon(nullptr);
			}
		}
	}
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

// Called to bind functionality to input
void AUK_CharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}


void AUK_CharacterBase::Attack()
{
	if ( 0 == CurrentComboCount )
	{
		BeginAttack();
	}
	else
	{
		ensure(FMath::IsWithinInclusive<int32>(CurrentComboCount, 1, MaxComboCount));
		bIsAttackKeyPressed = true;
	}
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

#pragma region Weapon
void AUK_CharacterBase::OnRep_CurrentWeapon(const AUK_WeaponBase* OldWeapon)
{
	if ( Weapon )
	{
		if ( !Weapon->GetOwnerCharactor() )
		{
			const FTransform PlacementTransform = Weapon->GetWeaponTransform() * GetMesh()->GetSocketTransform(FName("WeaponSocket"));
			Weapon->SetActorTransform(PlacementTransform, false, nullptr, ETeleportType::TeleportPhysics);
			Weapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepWorldTransform, FName("WeaponSocket"));
			Weapon->SetOwnerCharactor(this);
		}
		Weapon->GetStaticMeshComponent()->SetVisibility(true);
	}
}

#pragma endregion

#pragma region Attack

void AUK_CharacterBase::BeginAttack()
{
	TObjectPtr < UUK_AnimInstance > AnimInstance = Cast<UUK_AnimInstance>(GetMesh()->GetAnimInstance());

	bIsNowAttacking = true;
	if ( IsValid(AnimInstance) && IsValid(AttackMontage) && !( AnimInstance->Montage_IsPlaying(AttackMontage) ) )
	{
		AnimInstance->Montage_Play(AttackMontage);
	}

	CurrentComboCount = 1;

	if ( !OnMeleeAttackMontageEndedDelegate.IsBound() )
	{
		OnMeleeAttackMontageEndedDelegate.BindUObject(this, &ThisClass::EndAttack);
		AnimInstance->Montage_SetEndDelegate(OnMeleeAttackMontageEndedDelegate, AttackMontage);
	}
}

void AUK_CharacterBase::EndAttack(UAnimMontage* InMontage, bool bInterruped)
{
	ensureMsgf(CurrentComboCount != 0, TEXT("CurrentComboCount == 0"));

	CurrentComboCount = 0;
	bIsAttackKeyPressed = false;
	bIsNowAttacking = false;

	if ( OnMeleeAttackMontageEndedDelegate.IsBound() )
	{
		OnMeleeAttackMontageEndedDelegate.Unbind();
	}
}

void AUK_CharacterBase::HandleOnCheckHit()
{
	UKismetSystemLibrary::PrintString(this, TEXT("HandleOnCheckHit()"));

	TArray<FHitResult> HitResults;
	FCollisionQueryParams Params(NAME_None, false, this);
	FVector UpStartRange(30.f, 0.f, 0.f);
	FVector UpRange(30.f, 0.f, 40.f);

	bool bResult;
	if ( CurrentComboCount != 3 )
	{
		bResult = GetWorld()->SweepMultiByChannel(
			HitResults,
			GetActorLocation(),
			GetActorLocation() + AttackRange * GetActorForwardVector(),
			FQuat::Identity,
			ECC_ATTACK,
			FCollisionShape::MakeSphere(AttackRadius),
			Params
		);
		if ( ShowAttackDebug == 1 )
		{
			DrawSweepCapsuleDebug(
				AttackRange * GetActorForwardVector(),
				GetActorLocation() + AttackRange * GetActorForwardVector(),
				AttackRange * 0.5f + AttackRadius,
				bResult ? FColor::Green : FColor::Red
			);
		}
	}
	else
	{
		bResult = GetWorld()->SweepMultiByChannel(
			HitResults,
			AttackRange * GetActorForwardVector() + UpStartRange,
			GetActorLocation() + UpRange + ( AttackRange * GetActorForwardVector() ),
			FQuat::Identity,
			ECC_ATTACK,
			FCollisionShape::MakeSphere(AttackRadius),
			Params
		);
		if ( ShowAttackDebug == 1 )
		{
			DrawSweepCapsuleDebug(
				GetActorLocation() + UpStartRange,
				GetActorLocation() + UpRange + ( AttackRange * GetActorForwardVector() ),
				AttackRange * 0.5f + AttackRadius,
				bResult ? FColor::Green : FColor::Red
			);
		}
	}
	if ( bResult )
	{
		for ( FHitResult HitResult : HitResults )
		{
			if ( IsValid(HitResult.GetActor()) )
			{
				if ( 1 == ShowAttackDebug )
				{
					UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("Hit Actor Name: %s"), *HitResult.GetActor()->GetName()));
				}
			}
		}
	}
}
void AUK_CharacterBase::HandleOnCheckInputAttack()
{
	TObjectPtr < UUK_AnimInstance > AnimInstance = Cast<UUK_AnimInstance>(GetMesh()->GetAnimInstance());
	checkf(IsValid(AnimInstance), TEXT("Invalid AnimInstance"));

	if ( bIsAttackKeyPressed )
	{
		CurrentComboCount = FMath::Clamp(CurrentComboCount + 1, 1, MaxComboCount);

		FName NextSectionName = *FString::Printf(TEXT("%s%02d"), *MontageSectionName, CurrentComboCount);
		AnimInstance->Montage_JumpToSection(NextSectionName, AttackMontage);
		bIsAttackKeyPressed = false;
	}
}

void AUK_CharacterBase::DrawSweepCapsuleDebug(const FVector& Start, const FVector& End, float HalfHeight, const FColor& Color)
{
	const FVector Center = ( Start + End ) * 0.5f;
	FQuat CapsuleRot = FRotationMatrix::MakeFromZ(Center).ToQuat();

	DrawDebugCapsule(GetWorld(),
		Center,
		HalfHeight,
		AttackRadius,
		CapsuleRot,
		Color,
		false,
		5.f
	);
}

#pragma endregion
