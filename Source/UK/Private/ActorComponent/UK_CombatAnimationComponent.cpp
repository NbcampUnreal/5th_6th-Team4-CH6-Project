// Fill out your copyright notice in the Description page of Project Settings.


#include "ActorComponent/UK_CombatAnimationComponent.h"
#include "Character/UK_CharacterBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "DataAsset/UK_StatusAnimData.h"
#include "DataAsset/UK_WeaponData.h"
#include "DataAsset/UK_AnimData.h"
#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
UUK_CombatAnimationComponent::UUK_CombatAnimationComponent() :
	CurrentComboCount(0),
	DefaultGravityValue(1.f)
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	SetIsReplicatedByDefault(true);
}

void UUK_CombatAnimationComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UUK_CombatAnimationComponent, ServerComboCount);
	DOREPLIFETIME(UUK_CombatAnimationComponent, ServerAttackType);

}


// Called when the game starts
void UUK_CombatAnimationComponent::BeginPlay()
{
	Super::BeginPlay();
	OwnerCharactor = CastChecked<AUK_CharacterBase>(GetOwner());
}

// Called every frame
//void UUK_CombatAnimationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
//{
//	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
//}

void UUK_CombatAnimationComponent::PlayLightComboAnimation()
{
	UCharacterMovementComponent* PlayerMovement = OwnerCharactor->GetCharacterMovement();
	ensure(PlayerMovement);

	bool bPlayerIsFalling = PlayerMovement->IsFalling();
	if (CurrentComboCount == 0)
	{
		if (bPlayerIsFalling)
		{
			if (!OwnerCharactor->HasAuthority())
			{
				StartComboAttack(EComboAttackType::AttackOnAir);
			}

			ServerRPCStartComboAttack(EComboAttackType::AttackOnAir);
		}
		else
		{
			if (!OwnerCharactor->HasAuthority())
			{
				StartComboAttack(EComboAttackType::LightAttackOnGround);
			}
			ServerRPCStartComboAttack(EComboAttackType::LightAttackOnGround);
		}
	}
	else
	{
		if (!OwnerCharactor->HasAuthority())
		{
			if (!ComboCheckTimer.IsValid())
			{
				InputType = EAttackInput::Light;
			}
			else
			{
				InputType = EAttackInput::None;
			}
		}
	}
}

void UUK_CombatAnimationComponent::StartComboAttack(EComboAttackType AttackType)
{
	if ((!OwnerCharactor) || (!NowWeapon))
		return;

	AttackAnim = NowWeapon->FindAnimsDataAssetByType(AttackType);

	if (!IsValid(AttackAnim))
		return;

	CurrentComboCount = 1;
	FName ComboAttackMontageName = *FString::Printf(TEXT("%s%d"), *AttackAnim->MontageName, CurrentComboCount);

	PlayComboAttackAnimation(AttackType, ComboAttackMontageName);
	SetCheckComboTimer(AttackType);
}

void UUK_CombatAnimationComponent::PlayComboAttackAnimation(const EComboAttackType AttackType, FName SectionName)
{
	if (!OwnerCharactor)
		return;
	UAnimInstance* PlayerAnimInstance = OwnerCharactor->GetMesh()->GetAnimInstance();

	if (!IsValid(PlayerAnimInstance))
		return;
	TObjectPtr<UAnimMontage> ComboAttackMontage = AttackAnim->ComboMantage;
	if (!IsValid(ComboAttackMontage))
		return;

	if (!PlayerAnimInstance->Montage_IsPlaying(ComboAttackMontage))
	{
		if (AttackType == EComboAttackType::AttackOnAir)
		{
			StopJumpAndFly();
		}
		PlayerAnimInstance->Montage_Play(ComboAttackMontage);

		FOnMontageEnded EndDelegate;
		EndDelegate.BindUObject(this, &UUK_CombatAnimationComponent::EndComboAttack);
		PlayerAnimInstance->Montage_SetEndDelegate(EndDelegate, ComboAttackMontage);
	}
	PlayerAnimInstance->Montage_JumpToSection(SectionName, ComboAttackMontage);
}

void UUK_CombatAnimationComponent::SetCheckComboTimer(const EComboAttackType AttackType)
{
	int32 CurrentComboIndex = CurrentComboCount - 1;

	UAnimMontage* ComboAttackMontage = AttackAnim->ComboMantage;
	ensure(AttackAnim->ComboFrameTime[CurrentComboIndex]);

	float ComboAcceptTime = AttackAnim->ComboFrameTime[CurrentComboIndex];

	if (!FMath::IsNearlyZero(ComboAcceptTime))
	{
		FTimerDelegate ComboCheckDelegate;
		ComboCheckDelegate.BindUObject(this, &UUK_CombatAnimationComponent::CheckComboProcessable, AttackType);
		GetWorld()->GetTimerManager().SetTimer(
			ComboCheckTimer,
			ComboCheckDelegate,
			ComboAcceptTime,
			false
		);
	}
}

void UUK_CombatAnimationComponent::StopJumpAndFly()
{
	UCharacterMovementComponent* PlayerMovement = OwnerCharactor->GetCharacterMovement();
	ensure(PlayerMovement);

	PlayerMovement->GravityScale = 0.f;
	PlayerMovement->Velocity = FVector::ZeroVector;
	OwnerCharactor->StopJumping();

	PlayerMovement->SetJumpAllowed(false);
}

void UUK_CombatAnimationComponent::EndComboAttack(UAnimMontage* TargetMontage, bool bInterrupted)
{
	if (!bInterrupted)
	{
		ResetCharacterGravityScale();
		ResetPlayerComboAttackValue();
		ResetPlayerCharacterMovement();
	}
}

void UUK_CombatAnimationComponent::ResetCharacterGravityScale()
{
	if (!IsValid(OwnerCharactor))
		return;
	UCharacterMovementComponent* PlayerMovement = OwnerCharactor->GetCharacterMovement();
	PlayerMovement->GravityScale = DefaultGravityValue;
}

void UUK_CombatAnimationComponent::ResetPlayerComboAttackValue()
{
	ComboCheckTimer.Invalidate();
	LaunchplayerTimer.Invalidate();
	InputType = EAttackInput::None;

	CurrentComboCount = 0;
}

void UUK_CombatAnimationComponent::ResetPlayerCharacterMovement()
{
	UCharacterMovementComponent* PlayerMovement = OwnerCharactor->GetCharacterMovement();
	if (IsValid(PlayerMovement))
	{
		PlayerMovement->SetMovementMode(EMovementMode::MOVE_Walking);
		PlayerMovement->SetJumpAllowed(true);
	}
}

void UUK_CombatAnimationComponent::CheckComboProcessable(const EComboAttackType AttackType)
{
	ensure(IsValid(OwnerCharactor));

	++CurrentComboCount;

	EComboAttackType NextAttack = GetNextAttackType();
	if (AttackType != NextAttack)
	{
		CurrentComboCount = 1;
	}
	FName NextComboSectionName = *FString::Printf(TEXT("%s%d"), *AttackAnim->MontageName, CurrentComboCount);

	PlayComboAttackAnimation(NextAttack, NextComboSectionName);
	ServerRPCStartComboAttack(NextAttack);

	ComboCheckTimer.Invalidate();
	SetCheckComboTimer(NextAttack);

	InputType = EAttackInput::None;
}


EComboAttackType UUK_CombatAnimationComponent::GetNextAttackType()
{
	UCharacterMovementComponent* PlayerMovement = OwnerCharactor->GetCharacterMovement();
	bool bPlayerIsFalling = PlayerMovement->IsFalling();

	if (bPlayerIsFalling)
	{
		return EComboAttackType::AttackOnAir;
	}
	else
	{
		if(EAttackInput::Light == InputType)
		{
			return EComboAttackType::LightAttackOnGround;
		}
		else if (EAttackInput::Heavy == InputType)
		{
			return EComboAttackType::HeavyAttackOnGround;
		}
	}
	return EComboAttackType::None;
}

void UUK_CombatAnimationComponent::MulticastPlayCombo_Implementation(EComboAttackType AttackType, uint8 ComboCount)
{
	// AutonomousProxy는 로컬 예측을 이미 했으므로 스킵 가능
	if (OwnerCharactor->GetLocalRole() == ROLE_AutonomousProxy)
		return;

	AttackAnim = NowWeapon->FindAnimsDataAssetByType(AttackType);

	FName SectionName =
		*FString::Printf(TEXT("%s%d"), *AttackAnim->MontageName, ComboCount);

	PlayComboAttackAnimation(AttackType, SectionName);
}

void UUK_CombatAnimationComponent::ServerRPCStartComboAttack_Implementation(const EComboAttackType AttackType)
{
	if (!OwnerCharactor || !NowWeapon)
		return;

	if (ServerComboCount == 0)
	{
		ServerComboCount = 1;
		ServerAttackType = AttackType;
	}
	else
	{
		if (ServerAttackType != AttackType)
		{
			ServerComboCount = 1;
			ServerAttackType = AttackType;
		}
		else
		{
			++ServerComboCount;
		}
	}

	CurrentComboCount = ServerComboCount;

	MulticastPlayCombo(ServerAttackType, ServerComboCount);
}



