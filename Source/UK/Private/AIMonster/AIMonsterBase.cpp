#include "AIMonster/AIMonsterBase.h"
#include "AIController.h"
#include "Net/UnrealNetwork.h"
#include "AIMonster/UK_AiMonsterCtl.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Character/UK_CharacterBase.h"
#include "Kismet/GameplayStatics.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BrainComponent.h"
#include "TimerManager.h"

AAIMonsterBase::AAIMonsterBase()
{
	PrimaryActorTick.bCanEverTick = false; 

	bReplicates = true;
	SetReplicateMovement(true);
	StatComponent = CreateDefaultSubobject<UAI_MonsterStatComponent>(TEXT("StatComponent"));
	NetDormancy = DORM_DormantAll;
}

void AAIMonsterBase::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		SpawnLocation = GetActorLocation();

		if (Personality == EMonsterPersonality::Peaceful)
		{
			CurrentState = EMonsterState::Passive;
		}
	}

	if (StatComponent)
	{
		StatComponent->OnDeath.AddDynamic(this, &AAIMonsterBase::Die);
	}
}

// Tick() 제거
// 타겟 탐지: AIPerceptionComponent (이벤트 콜백)
// 타겟 갱신: BTService_DetectPlayer (0.5초 주기)
// AI 판단: BehaviorTree
// 공격: 몽타주 + AnimNotifyState (이벤트)
// 상태 전환: RequestState → OnStateChanged 델리게이트

/* =============================== */
/*         서버 상태 관리            */
/* =============================== */

void AAIMonsterBase::RequestState_Implementation(EMonsterState NewState)
{
	if (!HasAuthority()) return;
	if (CurrentState == NewState) return;
	SetServerState(NewState);
}

void AAIMonsterBase::SetServerState(EMonsterState NewState)
{
	EMonsterState OldState = CurrentState;
	CurrentState = NewState;

	OnStateChanged.Broadcast(OldState, NewState);
	OnRep_MonsterState();
}

void AAIMonsterBase::OnRep_MonsterState()
{
	// Dormancy만 관리 Tick 토글 제거
	switch (CurrentState)
	{
	case EMonsterState::Idle:
	case EMonsterState::Passive:
		SetNetDormancy(DORM_DormantAll);
		break;
	case EMonsterState::Dead:
		break;
	default:
		SetNetDormancy(DORM_Awake);
		break;
	}
}

void AAIMonsterBase::OnRep_IsAggressive()
{
	UE_LOG(LogTemp, Log, TEXT("[OnRep] %s bIsAggressive = %d"), *GetName(), bIsAggressive);
}

/* =============================== */
/*      몽타주 기반 공격 시스템       */
/* =============================== */

bool AAIMonsterBase::PlayRandomAttackMontage()
{
	if (bIsAttacking || bIsDying) return false;

	float Now = GetWorld()->GetTimeSeconds();
	if (Now - LastAttackTime < AttackCooldown) return false;

	if (AttackMontages.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Attack] %s: AttackMontages is empty!"), *GetName());
		return false;
	}

	int32 RandomIndex = FMath::RandRange(0, AttackMontages.Num() - 1);
	UAnimMontage* SelectedMontage = AttackMontages[RandomIndex];

	if (!SelectedMontage)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Attack] %s: Montage[%d] is null!"), *GetName(), RandomIndex);
		return false;
	}

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (!AnimInstance) return false;

	float MontageLength = AnimInstance->Montage_Play(SelectedMontage, 1.0f);
	if (MontageLength <= 0.f) return false;

	bIsAttacking = true;
	LastAttackTime = Now;

	FOnMontageEnded EndDelegate;
	EndDelegate.BindUObject(this, &AAIMonsterBase::OnAttackMontageEnded);
	AnimInstance->Montage_SetEndDelegate(EndDelegate, SelectedMontage);

	UE_LOG(LogTemp, Log, TEXT("[Attack] %s: Montage[%d] %s (%.2fs)"),
		*GetName(), RandomIndex, *SelectedMontage->GetName(), MontageLength);

	Multicast_PlayAttackMontage(RandomIndex);
	return true;
}

void AAIMonsterBase::Multicast_PlayAttackMontage_Implementation(int32 MontageIndex)
{
	if (HasAuthority()) return;
	if (!AttackMontages.IsValidIndex(MontageIndex)) return;

	UAnimMontage* Montage = AttackMontages[MontageIndex];
	if (!Montage) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (!AnimInstance) return;

	AnimInstance->Montage_Play(Montage, 1.0f);
}

void AAIMonsterBase::OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	bIsAttacking = false;
}

void AAIMonsterBase::Die()
{
	if (!HasAuthority()) return;
	if (CurrentState == EMonsterState::Dead || bIsDying) return;

	bIsDying = true;
	bIsAttacking = false;

	// BT 중지 + 이동 중지 (컨트롤러는 유지! 몽타주 재생에 필요)
	if (AAIController* AIC = Cast<AAIController>(GetController()))
	{
		AIC->StopMovement();

		if (UBrainComponent* Brain = AIC->GetBrainComponent())
		{
			Brain->StopLogic(TEXT("Dead"));
		}
	}

	// 몽타주를 먼저 재생 (아직 Awake 상태이므로 Multicast 전달됨)
	if (DeathMontage)
	{
		PlayDeathMontage();
	}

	// 그 다음 상태 변경 (Dead에서 Dormant 안 하므로 안전)
	SetServerState(EMonsterState::Dead);

	// 몽타주 없으면 딜레이 후 최종 처리
	if (!DeathMontage)
	{
		FTimerHandle DeathTimer;
		GetWorldTimerManager().SetTimer(
			DeathTimer, this, &AAIMonsterBase::FinalizeDeath,
			DeathWithoutMontageDelay, false);
	}
}

void AAIMonsterBase::PlayDeathMontage()
{
	if (!DeathMontage) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (!AnimInstance) return;

	AnimInstance->StopAllMontages(0.2f);

	float MontageLength = AnimInstance->Montage_Play(DeathMontage, 1.0f);
	if (MontageLength > 0.f)
	{
		FOnMontageEnded EndDelegate;
		EndDelegate.BindUObject(this, &AAIMonsterBase::OnDeathMontageEnded);
		AnimInstance->Montage_SetEndDelegate(EndDelegate, DeathMontage);
	}
	else
	{
		// 재생 실패 시 바로 마무리
		FinalizeDeath();
	}

	Multicast_PlayDeathMontage();
}

void AAIMonsterBase::Multicast_PlayDeathMontage_Implementation()
{
	if (HasAuthority()) return;
	if (!DeathMontage) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (!AnimInstance) return;

	AnimInstance->StopAllMontages(0.2f);
	AnimInstance->Montage_Play(DeathMontage, 1.0f);
}

void AAIMonsterBase::OnDeathMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	UE_LOG(LogTemp, Log, TEXT("[Death] %s: Death montage ended"), *GetName());
	FinalizeDeath();  // 몽타주 끝난 후에 최종 처리
}

// 사망 몽타주 끝난 후 호출
void AAIMonsterBase::FinalizeDeath()
{
	SetNetDormancy(DORM_DormantAll);
	DetachFromControllerPendingDestroy();
	OnDeath.Broadcast(this);
	UE_LOG(LogTemp, Log, TEXT("[Death] %s: FinalizeDeath complete"), *GetName());
}


/* =============================== */
/*          링크 시스템              */
/* =============================== */

void AAIMonsterBase::CallNearbyAllies(AActor* Enemy)
{
	if (!HasAuthority() || !Enemy) return;

	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), GetClass(), FoundActors);

	for (AActor* Actor : FoundActors)
	{
		if (!Actor || Actor == this) continue;

		AAIMonsterBase* Ally = Cast<AAIMonsterBase>(Actor);
		if (!Ally) continue;
		if (Ally->Personality != EMonsterPersonality::Peaceful) continue;
		if (Ally->bIsAggressive || Ally->IsDead()) continue;

		float Distance = FVector::Dist(GetActorLocation(), Ally->GetActorLocation());
		if (Distance > AllyCallRadius) continue;

		Ally->bIsAggressive = true;
		Ally->Aggressor = Enemy;
		Ally->RequestState(EMonsterState::Aggressive);

		if (AAIController* AllyAIC = Cast<AAIController>(Ally->GetController()))
		{
			if (UBlackboardComponent* BB = AllyAIC->GetBlackboardComponent())
			{
				BB->SetValueAsObject(TEXT("TargetPlayer"), Enemy);
			}
			if (UBehaviorTreeComponent* BTComp = Cast<UBehaviorTreeComponent>(AllyAIC->GetBrainComponent()))
			{
				BTComp->RestartTree();
			}
		}
	}
}

void AAIMonsterBase::ResetToPassive()
{
	if (!HasAuthority()) return;

	bIsAggressive = false;
	bIsAttacking = false;
	bIsDying = false;
	Aggressor = nullptr;

	if (StatComponent)
	{
		StatComponent->SetHP(StatComponent->GetMaxHP());
	}

	RequestState(EMonsterState::Passive);
}

/* =============================== */
/*           AI 활성화              */
/* =============================== */

void AAIMonsterBase::SetAIActive(bool bActive)
{
	/* 로직 수정 중 */
}

/* =============================== */
/*        Spawner / HP              */
/* =============================== */

void AAIMonsterBase::ResetHealth()
{
	if (!HasAuthority()) return;

	if (StatComponent)
	{
		StatComponent->SetHP(StatComponent->GetMaxHP());
	}

	bIsAttacking = false;
	bIsDying = false;

	if (Personality == EMonsterPersonality::Peaceful)
	{
		ResetToPassive();
	}
	else
	{
		SetServerState(EMonsterState::Idle);
	}
}

void AAIMonsterBase::ReceiveDamage(float Damage)
{
	if (!HasAuthority()) return;
	if (bIsDying) return;

	if (StatComponent)
	{
		float BeforeHp = StatComponent->GetHP();
		StatComponent->TakeDamage(Damage);

		UE_LOG(LogTemp, Warning,
			TEXT("[Monster Hit] %s | Dmg: %.1f | HP: %.1f -> %.1f"),
			*GetName(), Damage, BeforeHp, StatComponent->GetHP());
	}

	// 평화로운 몬스터가 처음 공격받았을 때
	if (Personality == EMonsterPersonality::Peaceful && !bIsAggressive)
	{
		AActor* ClosestPlayer = nullptr;
		
		for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
		{
			APlayerController* PC = It->Get();
			if (!PC || !PC->GetPawn()) continue;

			float Dist = FVector::Dist(GetActorLocation(), PC->GetPawn()->GetActorLocation());
			if (Dist <= 500.0f)
			{
				ClosestPlayer = PC->GetPawn();
				break;
			}
		}

		if (ClosestPlayer)
		{
			bIsAggressive = true;
			Aggressor = ClosestPlayer;

			if (AAIController* AIC = Cast<AAIController>(GetController()))
			{
				if (UBlackboardComponent* BB = AIC->GetBlackboardComponent())
				{
					BB->SetValueAsObject(TEXT("TargetPlayer"), ClosestPlayer);
				}
			}

			CallNearbyAllies(ClosestPlayer);
			OnAttacked.Broadcast(this, ClosestPlayer);
			RequestState(EMonsterState::Aggressive);

			if (AAIController* AIC = Cast<AAIController>(GetController()))
			{
				if (UBehaviorTreeComponent* BTComp = Cast<UBehaviorTreeComponent>(AIC->GetBrainComponent()))
				{
					BTComp->RestartTree();
				}
			}
		}
	}
}

/* =============================== */
/*       상태별 가상 함수            */
/* =============================== */

void AAIMonsterBase::OnIdle() {}
void AAIMonsterBase::OnPatrol() {}
void AAIMonsterBase::OnChase(float DeltaSeconds) {}

void AAIMonsterBase::OnAttack()
{
	// 데미지는 AnimNotifyState_MonsterMeleeTrace에서 트레이스 처리
	if (!HasAuthority()) return;
	PlayRandomAttackMontage();
}

void AAIMonsterBase::OnDead() {}
void AAIMonsterBase::OnPassive() {}
void AAIMonsterBase::OnAlert() {}

/* =============================== */
/*          Replication             */
/* =============================== */

void AAIMonsterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AAIMonsterBase, CurrentState);
	DOREPLIFETIME(AAIMonsterBase, bIsAggressive);
	DOREPLIFETIME(AAIMonsterBase, Aggressor);
}