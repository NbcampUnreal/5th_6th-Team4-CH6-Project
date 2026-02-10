// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/UK_CharacterBase.h"
#include "Character/UK_PlayerController.h"
#include "Character/UK_PlayerState.h"
#include "Character/Weapon/UK_WeaponBase.h"
#include "Tags/UK_GameplayTags.h"
#include "Animation/UK_AnimInstance.h"
#include "ActorComponent/StatusComponent.h"
#include "ActorComponent/UK_CombatAnimationComponent.h"
#include "ActorComponent/UK_InventoryComponent.h"
#include "DataAsset/UK_WeaponData.h"
#include "DataAsset/UK_StatusAnimData.h"
#include "DataAsset/Data/UK_ItemData.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "AbilitySystemComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Perception/AISense_Sight.h"
#include "Net/UnrealNetwork.h"

#pragma region Defualt



// Sets default values
AUK_CharacterBase::AUK_CharacterBase() :
	NowWeapon(UK_GameplayTags::Weapon::WeaponRoot)
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

	MannySkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MannySkeletalMesh"));
	MannySkeletalMesh->SetupAttachment(GetMesh());
	ChildActorComponent = CreateDefaultSubobject<UChildActorComponent>(TEXT("ChildActorComponent"));
	ChildActorComponent->SetupAttachment(MannySkeletalMesh, TEXT("Weapon"));


	StimuliSource = CreateDefaultSubobject<UAIPerceptionStimuliSourceComponent>(TEXT("StimuliSource"));
	StatusComponent = CreateDefaultSubobject<UStatusComponent>(TEXT("StatusComponent"));
	InventoryComponent = CreateDefaultSubobject<UUK_InventoryComponent>(TEXT("InventoryComponent"));
	AnimationComponent = CreateDefaultSubobject<UUK_CombatAnimationComponent>(TEXT("AnimComponent"));
}

void AUK_CharacterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AUK_CharacterBase, StatusComponent, COND_None);

}

// Called when the game starts or when spawned
void AUK_CharacterBase::BeginPlay()
{
	Super::BeginPlay();

	StatusComponent->OnDeadDelegate.AddDynamic(this, &AUK_CharacterBase::Dead);
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

void AUK_CharacterBase::Attack()
{
	if ( StatusComponent->IsDead() )
	{
		return;
	}
	AnimationComponent->PlayLightComboAnimation();
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
// 어느 타이밍에 호출할지 고민 필요
#pragma endregion

#pragma region Weapon


void AUK_CharacterBase::EquipWeapon(FGameplayTag NewWeapon)
{

	GetAbilitySystemComponent()->RemoveLooseGameplayTag(NowWeapon);
	NowWeapon = NewWeapon;
	UUK_StatusAnimData* Weapon = WeaponList->FindAnimsDataAssetByTag(NowWeapon);
	AnimationComponent->SetNowWeapon(Weapon);
	GetAbilitySystemComponent()->AddLooseGameplayTag(NowWeapon);
}
void AUK_CharacterBase::SlotWeaponOne()
{
	SwapWeapon(0);
}
void AUK_CharacterBase::SlotWeaponTwo()
{
	SwapWeapon(1);
}
void AUK_CharacterBase::SlotWeaponThree()
{
	SwapWeapon(2);
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
	AnimationComponent->SetNowWeapon(WeaponList->FindAnimsDataAssetByTag(NowWeapon));
}
#pragma endregion

#pragma region Battle

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
#pragma endregion