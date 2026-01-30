// Fill out your copyright notice in the Description page of Project Settings.


#include "ActorComponent/UK_CombatAnimationComponent.h"
#include "Character/UK_CharacterBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "DataAsset/UK_StatusAnimData.h"
#include "DataAsset/UK_AnimData.h"
#include "Net/UnrealNetwork.h"
int32 UUK_CombatAnimationComponent::ShowAttackDebug = 0;

FAutoConsoleVariableRef CVarShowAttackDebug(
	TEXT("UK.ShowAttackDebug"),
	UUK_CombatAnimationComponent::ShowAttackDebug,
	TEXT(""),
	ECVF_Cheat
);
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

// 진입점
void UUK_CombatAnimationComponent::PlayLightComboAnimation() 
{
	if ( !IsValid(NowWeapon) )
		return;

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

// 콤보 최초 시작
void UUK_CombatAnimationComponent::StartComboAttack(EComboAttackType AttackType) 
{
	if ((!OwnerCharactor) || (!NowWeapon))
		return;

	// 재생될 애님 에셋을 가져옴
	AttackAnim = NowWeapon->FindAnimsDataAssetByType(AttackType); 

	if (!IsValid(AttackAnim))
		return;

	CurrentComboCount = 1;
	FName ComboAttackMontageName = *FString::Printf(TEXT("%s%d"), *AttackAnim->MontageName, CurrentComboCount);

	// 애니메이션  플레이
	PlayComboAttackAnimation(AttackType, ComboAttackMontageName);
	// 다음 콤보 체크 타이머 시작
	SetCheckComboTimer(AttackType);
}

void UUK_CombatAnimationComponent::PlayComboAttackAnimation(const EComboAttackType AttackType, FName SectionName)
{
	if (!OwnerCharactor)
		return;
	if ( !IsValid(NowWeapon) )
		return;
	UAnimInstance* PlayerAnimInstance = OwnerCharactor->GetMesh()->GetAnimInstance();

	if (!IsValid(PlayerAnimInstance))
		return;

	// 플레이될 몽타주 가져옴
	TObjectPtr<UAnimMontage> ComboAttackMontage = AttackAnim->ComboMantage;

	if (!IsValid(ComboAttackMontage))
		return;

	// 몽타주 재생이 안되고 있을 때만 진입
	if (!PlayerAnimInstance->Montage_IsPlaying(ComboAttackMontage))
	{
		if (AttackType == EComboAttackType::AttackOnAir)
		{
			// 공중 공격시 공중에 유지
			StopJumpAndFly();
		}
		// 애니메이션 재생
		PlayerAnimInstance->Montage_Play(ComboAttackMontage);

		FOnMontageEnded EndDelegate;

		//애니메이션이 끝나면 자동으로 종료
		EndDelegate.BindUObject(this, &UUK_CombatAnimationComponent::EndComboAttack);
		PlayerAnimInstance->Montage_SetEndDelegate(EndDelegate, ComboAttackMontage);
	}
	// 출력될 애니메이션 섹션으로 점프
	PlayerAnimInstance->Montage_JumpToSection(SectionName, ComboAttackMontage);
}

//타이머 세팅
void UUK_CombatAnimationComponent::SetCheckComboTimer(const EComboAttackType AttackType)
{
	if ( !IsValid(NowWeapon) )
		return;

	if ( AttackAnim->MaxComboCount == CurrentComboCount )
		return;

	int32 CurrentComboIndex = CurrentComboCount - 1;

	UAnimMontage* ComboAttackMontage = AttackAnim->ComboMantage;
	ensure(AttackAnim->ComboFrameTime[CurrentComboIndex]);

	// 타이머 타임을 가져옴
	float ComboAcceptTime = AttackAnim->ComboFrameTime[CurrentComboIndex];

	if (ComboAcceptTime != 0.f)
	{

		FTimerDelegate ComboCheckDelegate;
		// 타이머가 끝나면 후속타 점검
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

void UUK_CombatAnimationComponent::CheckComboProcessable(const EComboAttackType AttackType)
{
	if ( !IsValid(NowWeapon) )
		return;
	// 타이머 초기화
	ComboCheckTimer.Invalidate();

	ensure(IsValid(OwnerCharactor));
	// 입력 감지에 안된다면 콤보 재생종료
	if ( InputType == EAttackInput::None )
		return;
	++CurrentComboCount;

	EComboAttackType NextAttack = GetNextAttackType();
	// 공격 타입이 달라진다면
	if (AttackType != NextAttack)
	{
		//콤보를 처음부터 시작
		CurrentComboCount = 1;
	}

	FName NextComboSectionName = *FString::Printf(TEXT("%s%d"), *AttackAnim->MontageName, CurrentComboCount);
	
	// 애니메이션 재생
	PlayComboAttackAnimation(NextAttack, NextComboSectionName);
	ServerRPCStartComboAttack(NextAttack);

	// 다음 콤보 체크 타이머
	SetCheckComboTimer(NextAttack);

	InputType = EAttackInput::None;
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

// 현재 상태에 따른 어택타입 가져오기
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
	if (OwnerCharactor->GetLocalRole() == ROLE_AutonomousProxy)
		return;

	AttackAnim = NowWeapon->FindAnimsDataAssetByType(AttackType);

	FName SectionName = *FString::Printf(TEXT("%s%d"), *AttackAnim->MontageName, ComboCount);

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

void UUK_CombatAnimationComponent::SetEnableHitCheck(bool bEnablaHitCheck)
{
	if ( bEnablaHitCheck )
	{
		GetWorld()->GetTimerManager().SetTimer
		(
			HitCheckTimer,
			this, 
			&UUK_CombatAnimationComponent::HitCheckProcess,
			0.1f,
			true
		);
	}
	else
	{
		GetWorld()->GetTimerManager().ClearTimer(HitCheckTimer);
		HitcheckedActor.Empty();
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
		FColor DrawColor = bIsHit ? FColor::Red : FColor::Green;
		DrawDebugCapsule(
			GetWorld(),
			(TraceStart + TraceEnd) / 2,
			(TraceEnd - TraceStart).Size() / 2,
			CapsuleRadius,
			FQuat::Identity,
			DrawColor,
			false,
			1.f
		);
#endif
	}

	if (bIsHit)
	{
		for (const FHitResult& Hit : HitResult)
		{
			
			AActor* HitActor = Hit.GetActor();
			bool bAlreadyHit = HitcheckedActor.Contains(HitActor);
			if ( !bAlreadyHit )
			{
				HitcheckedActor.Add(HitActor);

				UE_LOG(LogTemp, Warning, TEXT("Hit Actor: %s"), *HitActor->GetName());
				if ( OwnerCharactor->HasAuthority() )
				{
					// 데미지 로직
					HitActor->TakeDamage(10.f, DamageEvent, OwnerCharactor->GetController(), OwnerCharactor);
				}
				if ( OwnerCharactor->IsLocallyControlled() )
				{
					// 이펙트 출력
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



