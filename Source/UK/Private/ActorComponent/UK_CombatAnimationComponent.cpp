// Fill out your copyright notice in the Description page of Project Settings.


#include "ActorComponent/UK_CombatAnimationComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Character/UK_CharacterBase.h"
#include "AIMonster/AIMonsterBase.h"
#include "DataAsset/UK_StatusAnimData.h"
#include "DataAsset/UK_AnimData.h"
#include "Animation/AnimMontage.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundAttenuation.h"
int32 UUK_CombatAnimationComponent::ShowAttackDebug = 0;

FAutoConsoleVariableRef CVarShowAttackDebug(
	TEXT("UK.ShowAttackDebug"),
	UUK_CombatAnimationComponent::ShowAttackDebug,
	TEXT(""),
	ECVF_Cheat
);
// Sets default values for this component's properties

#pragma region Defualt

UUK_CombatAnimationComponent::UUK_CombatAnimationComponent() :
	CurrentComboCount(0),
	DefaultGravityValue(1.f)
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	SetIsReplicatedByDefault(true);
}

// Called every frame
//void UUK_CombatAnimationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
//{
//	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
//}

void UUK_CombatAnimationComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UUK_CombatAnimationComponent, CurrentComboCount);

}

// Called when the game starts
void UUK_CombatAnimationComponent::BeginPlay()
{
	Super::BeginPlay();
	OwnerCharactor = Cast<AUK_CharacterBase>(GetOwner());
}
#pragma endregion

void UUK_CombatAnimationComponent::OnRep_CurrentComboCount()
{
}

// 진입점
void UUK_CombatAnimationComponent::PlayLightComboAnimation()
{
	if ( !IsValid(NowWeapon) || !IsValid(OwnerCharactor) )
		return;

	UCharacterMovementComponent* PlayerMovement = OwnerCharactor->GetCharacterMovement();

	ensure(PlayerMovement);

	bool bPlayerIsFalling = PlayerMovement->IsFalling();

	if ( CurrentComboCount == 0 )
	{
		if ( bPlayerIsFalling )
		{
			ServerRPCStartComboAttack(EComboAttackType::AttackOnAir);
		}
		else
		{
			ServerRPCStartComboAttack(EComboAttackType::LightAttackOnGround);
		}
	}
	else
	{
		InputType = EAttackInput::Light;
	}
}

#pragma region ServerRPCs
// 콤보 최초 시작
void UUK_CombatAnimationComponent::ServerRPCStartComboAttack_Implementation(const EComboAttackType AttackType)
{
	if ( !OwnerCharactor || !NowWeapon )
		return;

	CurrentComboCount = 1;

	MulticastPlayCombo(AttackType, CurrentComboCount);
}

void UUK_CombatAnimationComponent::ServerRPCComboAttack_Implementation(const EComboAttackType AttackType, FName SectionName)
{
	if ( !NowWeapon ) return;

	EComboAttackType NextAttack = GetNextAttackType();

	CurrentComboCount++;

	MulticastPlayCombo(AttackType, CurrentComboCount);
}

void UUK_CombatAnimationComponent::MulticastPlayCombo_Implementation(EComboAttackType AttackType, uint8 ComboCount)
{

	AttackAnim = NowWeapon->FindAnimsDataAssetByType(AttackType);

	FName SectionName = *FString::Printf(TEXT("%s%d"), *AttackAnim->MontageSectionName, ComboCount);

	PlayComboAttackAnimation(AttackType, SectionName);
}
#pragma endregion

void UUK_CombatAnimationComponent::PlayComboAttackAnimation(const EComboAttackType AttackType, FName SectionName)
{
	if ( !OwnerCharactor )
		return;
	if ( !IsValid(NowWeapon) )
		return;
	UAnimInstance* PlayerAnimInstance = OwnerCharactor->GetMesh()->GetAnimInstance();

	if ( !IsValid(PlayerAnimInstance) )
		return;

	// 플레이될 몽타주 가져옴
	TObjectPtr<UAnimMontage> ComboAttackMontage = AttackAnim->ComboMantage;

	if ( !IsValid(ComboAttackMontage) )
	{
		EndComboAttack(ComboAttackMontage, false);
		return;
	}

	// 몽타주 재생이 안되고 있을 때만 진입
	if ( !PlayerAnimInstance->Montage_IsPlaying(ComboAttackMontage) )
	{
		if ( AttackType == EComboAttackType::AttackOnAir )
		{
			// 공중 공격시 공중에 유지
			StopJumpAndFly();
		}
		// 애니메이션 재생
		PlayerAnimInstance->Montage_Play(ComboAttackMontage);

		FOnMontageBlendingOutStarted EndDelegate;

		//애니메이션이 끝나면 자동으로 종료
		EndDelegate.BindUObject(this, &UUK_CombatAnimationComponent::EndComboAttack);
		PlayerAnimInstance->Montage_SetEndDelegate(EndDelegate, ComboAttackMontage);
	}
	// 출력될 애니메이션 섹션으로 점프
	PlayerAnimInstance->Montage_JumpToSection(SectionName, ComboAttackMontage);
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

#pragma region EndCombo
void UUK_CombatAnimationComponent::EndComboAttack(UAnimMontage* TargetMontage, bool bInterrupted)
{
	if ( !bInterrupted )
	{
		UE_LOG(LogTemp, Display, TEXT("EndComboAttack()"));
		ResetCharacterGravityScale();
		//ServerResetPlayerComboAttackValue();
		ResetPlayerComboAttackValue();
		ResetPlayerCharacterMovement();
	}
}

void UUK_CombatAnimationComponent::ResetCharacterGravityScale()
{
	if ( !IsValid(OwnerCharactor) )
	{
		UE_LOG(LogTemp, Display, TEXT("ResetCharacterGravityScale()return"));
		return;
	}
	UE_LOG(LogTemp, Display, TEXT("Reset()"));
	UCharacterMovementComponent* PlayerMovement = OwnerCharactor->GetCharacterMovement();
	PlayerMovement->GravityScale = DefaultGravityValue;
}

void UUK_CombatAnimationComponent::ResetPlayerComboAttackValue()
{
	InputType = EAttackInput::None;

	CurrentComboCount = 0;
}

void UUK_CombatAnimationComponent::ResetPlayerCharacterMovement()
{
	UCharacterMovementComponent* PlayerMovement = OwnerCharactor->GetCharacterMovement();
	if ( IsValid(PlayerMovement) )
	{
		PlayerMovement->SetMovementMode(EMovementMode::MOVE_Walking);
		PlayerMovement->SetJumpAllowed(true);
	}
}
#pragma endregion

void UUK_CombatAnimationComponent::CheckComboProcessable(const EComboAttackType AttackType)
{
	if ( !IsValid(NowWeapon) )
		return;

	ensure(IsValid(OwnerCharactor));
	//// 입력 감지에 안된다면 콤보 재생종료
	if ( InputType == EAttackInput::None )
	{
		TObjectPtr<UAnimMontage> ComboAttackMontage = AttackAnim->ComboMantage;

		EndComboAttack(ComboAttackMontage, false);
		return;
	}

	FName NextComboSectionName = *FString::Printf(TEXT("%s%d"), *AttackAnim->MontageSectionName, CurrentComboCount);

	// 애니메이션 재생
	ServerRPCComboAttack(AttackType, NextComboSectionName);

	InputType = EAttackInput::None;
}

// 현재 상태에 따른 어택타입 가져오기
EComboAttackType UUK_CombatAnimationComponent::GetNextAttackType()
{
	UCharacterMovementComponent* PlayerMovement = OwnerCharactor->GetCharacterMovement();
	bool bPlayerIsFalling = PlayerMovement->IsFalling();

	if ( bPlayerIsFalling )
	{
		return EComboAttackType::AttackOnAir;
	}
	else
	{
		if ( EAttackInput::Light == InputType )
		{
			return EComboAttackType::LightAttackOnGround;
		}
		else if ( EAttackInput::Heavy == InputType )
		{
			return EComboAttackType::HeavyAttackOnGround;
		}
	}
	return EComboAttackType::None;
}

#pragma region Battle
void UUK_CombatAnimationComponent::SetEnableHitCheck(bool bEnablaHitCheck)
{
	if ( bEnablaHitCheck )
	{
		HitcheckedActor.Reset();
		GetWorld()->GetTimerManager().SetTimer
		(
			HitCheckTimer,
			this,
			&UUK_CombatAnimationComponent::HitCheckProcess,
			0.1f,
			true
		);
		TObjectPtr<USoundBase> AttackSound = AttackAnim->AttackSound;
		ServerRPCPlaySoundAndEffect(AttackSound);
	}
	else
	{
		GetWorld()->GetTimerManager().ClearTimer(HitCheckTimer);
		HitcheckedActor.Reset();
		HitCheckTimer.Invalidate();
	}
}

void UUK_CombatAnimationComponent::HitCheckProcess()
{
	if ( !IsValid(WeaponMesh) )
		return;
	FVector TraceStart = WeaponMesh->GetSocketLocation(TraceStartSocketName);
	FVector TraceEnd = WeaponMesh->GetSocketLocation(TraceEndSocketName);

	const float CapsuleRadius = 50.f;

	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(OwnerCharactor);
	CollisionParams.bReturnPhysicalMaterial = true;

	TArray<FHitResult> HitResult;
	FCollisionShape CollisionShape = FCollisionShape::MakeSphere(CapsuleRadius);

	bool bIsHit = GetWorld()->SweepMultiByChannel(
		HitResult,
		TraceStart,
		TraceEnd,
		FQuat::Identity,
		ECC_ATTACK,
		CollisionShape,
		CollisionParams
	);
	if ( ShowAttackDebug )
	{
#if ENABLE_DRAW_DEBUG
		FColor DrawColor = bIsHit ? FColor::Green : FColor::Red;

		FQuat CapsuleRot = FRotationMatrix::MakeFromZ(TraceEnd - TraceStart).ToQuat();
		DrawDebugCapsule(
			GetWorld(),
			( TraceStart + TraceEnd ) / 2,
			( TraceEnd - TraceStart ).Size(),
			CapsuleRadius,
			CapsuleRot,
			DrawColor,
			false,
			1.f
		);
#endif
	}

	if ( bIsHit )
	{
		for ( const FHitResult& Hit : HitResult )
		{

			AActor* HitActor = Hit.GetActor();
			bool bAlreadyHit = HitcheckedActor.Contains(HitActor);
			if ( !bAlreadyHit )
			{
				HitcheckedActor.Add(HitActor);

				UE_LOG(LogTemp, Warning, TEXT("Hit Actor: %s"), *HitActor->GetName());
				if ( OwnerCharactor->HasAuthority() )
				{
					if ( TObjectPtr<AAIMonsterBase> Monster = Cast<AAIMonsterBase>(HitActor) )
					{
						Monster->ReceiveDamage(OwnerCharactor->ApplyDamage());
						UE_LOG(LogTemp, Warning, TEXT("Damage Applied to Monster: %s to Damage : %f"), *Monster->GetName(), OwnerCharactor->ApplyDamage());
					}
					// 이펙트 출력
					TObjectPtr<USoundBase> HitSound = AttackAnim->HitSound;
					ServerRPCPlaySoundAndEffect(HitSound);
				}
			}
		}
	}
}

void UUK_CombatAnimationComponent::SetWeaponMesh(UStaticMeshComponent* NewWeapon)
{
	if ( !IsValid(NewWeapon) )
		return;
	WeaponMesh = NewWeapon;
}

void UUK_CombatAnimationComponent::ServerRPCPlaySoundAndEffect_Implementation(USoundBase* Sound)
{
	MulticastPlaySoundAndEffect(Sound);
}

void UUK_CombatAnimationComponent::MulticastPlaySoundAndEffect_Implementation(USoundBase* Sound)
{
	if ( IsValid(Sound) && IsValid(SoundAttenuation) )
	{
		FVector Start = WeaponMesh->GetSocketLocation(TraceStartSocketName);
		FVector End = WeaponMesh->GetSocketLocation(TraceEndSocketName);
		UGameplayStatics::PlaySoundAtLocation(
			GetWorld(),
			Sound,
			( Start + End ) / 2.f,
			1.f,
			1.f,
			0.f,
			SoundAttenuation
		);
	}
}

void UUK_CombatAnimationComponent::SetNowWeapon(const TObjectPtr<UUK_StatusAnimData>& Weapon)
{
	if ( IsValid(Weapon) )
	{
		NowWeapon = Weapon;
	}
}
#pragma endregion
