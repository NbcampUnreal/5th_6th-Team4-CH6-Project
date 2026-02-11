#include "AIMonster/AIMonsterBase.h"
#include "AIController.h"
#include "Net/UnrealNetwork.h"
#include "AIMonster/UK_AiMonsterCtl.h"
#include "Character/UK_CharacterBase.h"
#include "Kismet/GameplayStatics.h"
#include "BehaviorTree/BlackboardComponent.h"

AAIMonsterBase::AAIMonsterBase()
{
	PrimaryActorTick.bCanEverTick = true;

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
		SetActorTickEnabled(false);
		SpawnLocation = GetActorLocation();
		
		// 평화로운 몬스터는 Passive 상태로 시작
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

void AAIMonsterBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

    if (!HasAuthority()) return;
}

/* 서버로 상태 요청을 보내는 구간 */

void AAIMonsterBase::RequestState_Implementation(EMonsterState NewState)
{
    if (!HasAuthority()) return;
    if (CurrentState == NewState) return;

    SetServerState(NewState);
}

void AAIMonsterBase::SetServerState(EMonsterState NewState)
{
    CurrentState = NewState;

    if (CurrentState == EMonsterState::Dead)
    {
        DetachFromControllerPendingDestroy();
    }

    OnRep_MonsterState();
}

void AAIMonsterBase::OnRep_MonsterState()
{
    switch (CurrentState)
    {
    case EMonsterState::Idle:
        SetActorTickEnabled(false);
        SetNetDormancy(DORM_DormantAll);
        break;

    case EMonsterState::Patrol:
        SetActorTickEnabled(true);
        PrimaryActorTick.TickInterval = TickIntervalPatrol;
        SetNetDormancy(DORM_Awake);
        break;

    case EMonsterState::Chase:
        SetActorTickEnabled(true);
        PrimaryActorTick.TickInterval = TickIntervalChase;
        SetNetDormancy(DORM_Awake);
        break;

    case EMonsterState::Attack:
        SetActorTickEnabled(true);
        PrimaryActorTick.TickInterval = TickIntervalAttack;
        SetNetDormancy(DORM_Awake);
        break;

    case EMonsterState::Dead:
        SetActorTickEnabled(false);
        SetNetDormancy(DORM_DormantAll);
        break;
        
    case EMonsterState::Passive:
        SetActorTickEnabled(true);
        PrimaryActorTick.TickInterval = TickIntervalPassive;
        SetNetDormancy(DORM_Awake);
        break;
        
    case EMonsterState::Alert:
        SetActorTickEnabled(true);
        PrimaryActorTick.TickInterval = TickIntervalAlert;
        SetNetDormancy(DORM_Awake);
        break;
        
    case EMonsterState::Aggressive:
        SetActorTickEnabled(true);
        PrimaryActorTick.TickInterval = TickIntervalChase;
        SetNetDormancy(DORM_Awake);
        break;
    }
}

/* 링크 시스템 - 주변 동료들 부르기 */
void AAIMonsterBase::CallNearbyAllies(AActor* Enemy)
{
	if (!HasAuthority()) return;
	if (!Enemy) return;
	
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), GetClass(), FoundActors);
	
	for (AActor* Actor : FoundActors)
	{
		if (!Actor || Actor == this) continue;
		
		AAIMonsterBase* AllyMonster = Cast<AAIMonsterBase>(Actor);
		if (!AllyMonster) continue;
		
		// 같은 성격의 몬스터만
		if (AllyMonster->Personality != EMonsterPersonality::Peaceful) continue;
		
		// 이미 적대적이면 패스
		if (AllyMonster->bIsAggressive) continue;
		
		// 죽었으면 패스
		if (AllyMonster->IsDead()) continue;
		
		// 거리 체크
		float Distance = FVector::Dist(GetActorLocation(), AllyMonster->GetActorLocation());
		if (Distance > AllyCallRadius) continue;
		
		// 동료 몬스터를 적대적으로 만들기
		AllyMonster->bIsAggressive = true;
		AllyMonster->Aggressor = Enemy;
		AllyMonster->RequestState(EMonsterState::Aggressive);
		
		UE_LOG(LogTemp, Log, TEXT("[Link System] %s called ally %s to fight!"), 
			*GetName(), *AllyMonster->GetName());
	}
}

/* 복귀 시스템 - 원래 상태로 돌아가기 */
void AAIMonsterBase::ResetToPassive()
{
	if (!HasAuthority()) return;
	
	bIsAggressive = false;
	Aggressor = nullptr;
	
	// 체력 회복
	if (StatComponent)
	{
		StatComponent->SetHP(StatComponent->GetMaxHP());
		UE_LOG(LogTemp, Log, TEXT("[Reset] %s reset to passive and healed"), *GetName());
	}
	
	RequestState(EMonsterState::Passive);
}

/* AI 활성화 제어 스위치 같은 역할 */

void AAIMonsterBase::SetAIActive(bool bActive)
{
	/* 로직 수정 중 */
}

/* Spawner System*/
void AAIMonsterBase::Die()
{
	if (!HasAuthority()) return;
	if (IsDead()) return;

	SetServerState(EMonsterState::Dead);
	OnDeath.Broadcast(this);
}

void AAIMonsterBase::ResetHealth()
{
	if (HasAuthority())
	{
		if (StatComponent)
		{
			StatComponent->SetHP(StatComponent->GetMaxHP());
		}
		
		// 평화로운 몬스터는 Passive로 리셋
		if (Personality == EMonsterPersonality::Peaceful)
		{
			ResetToPassive();
		}
		else
		{
			SetServerState(EMonsterState::Idle);
		}
	}
}

void AAIMonsterBase::ReceiveDamage(float Damage)
{
	if ( !HasAuthority() ) return;

	if ( StatComponent )
	{
		float BeforeHp = StatComponent->GetHP();

		StatComponent->TakeDamage(Damage);

		float AfterHp = StatComponent->GetHP();

		UE_LOG(LogTemp, Warning,
			TEXT("[Monster Hit] %s | Damage: %.1f | HP: %.1f -> %.1f"),
			*GetName(),
			Damage,
			BeforeHp,
			AfterHp
		);
	}
	
	// 평화로운 몬스터가 처음 공격받았을 때
	if (Personality == EMonsterPersonality::Peaceful && !bIsAggressive)
	{
		// 플레이어 직접 가져오기 (가장 확실한 방법)
		AActor* ClosestPlayer = nullptr;
		
		// 방법 1: PlayerController를 통해 플레이어 가져오기
		APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
		if (PC && PC->GetPawn())
		{
			APawn* PlayerPawn = PC->GetPawn();
			float Distance = FVector::Dist(GetActorLocation(), PlayerPawn->GetActorLocation());
			
			// 500 유닛 안에 있으면 (공격 범위 추정)
			if (Distance <= 500.0f)
			{
				ClosestPlayer = PlayerPawn;
			}
		}
		
		// 방법 2: PlayerController 실패 시 백업 (AI Controller가 아닌 Character 찾기)
		if (!ClosestPlayer)
		{
			TArray<AActor*> FoundActors;
			UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACharacter::StaticClass(), FoundActors);
			
			float ClosestDist = 500.0f;
			
			for (AActor* Actor : FoundActors)
			{
				if (!Actor || Actor == this) continue;
				
				// AI Controller를 가진 Character는 제외 (몬스터)
				if (ACharacter* Char = Cast<ACharacter>(Actor))
				{
					if (Cast<AAIController>(Char->GetController()))
						continue;  // AI Controller면 몬스터이므로 제외
				}
				
				float Dist = FVector::Dist(GetActorLocation(), Actor->GetActorLocation());
				if (Dist < ClosestDist)
				{
					ClosestDist = Dist;
					ClosestPlayer = Actor;
				}
			}
		}
		
		if (ClosestPlayer)
		{
			bIsAggressive = true;
			Aggressor = ClosestPlayer;
			
			UE_LOG(LogTemp, Warning, TEXT("[Peaceful Monster] %s became aggressive!"), *GetName());
			
			// Blackboard에 즉시 타겟 설정 (추격을 위해)
			AAIController* AIController = Cast<AAIController>(GetController());
			if (AIController)
			{
				UBlackboardComponent* BlackboardComp = AIController->GetBlackboardComponent();
				if (BlackboardComp)
				{
					BlackboardComp->SetValueAsObject(TEXT("TargetPlayer"), ClosestPlayer);
					UE_LOG(LogTemp, Log, TEXT("[Peaceful Monster] TargetPlayer set to %s"), *ClosestPlayer->GetName());
				}
			}
			
			// 주변 동료들 부르기
			CallNearbyAllies(ClosestPlayer);
			
			// 공격받았다는 이벤트 발생
			OnAttacked.Broadcast(this, ClosestPlayer);
			
			// 상태 변경
			RequestState(EMonsterState::Aggressive);
		}
	}
}

/* 상태별 기본적인 동작 혹은 행위 */
/* 자식 클래스에서 상속 받아서 사용 될 함수 */

void AAIMonsterBase::OnIdle()
{
}

void AAIMonsterBase::OnPatrol()
{
}

void AAIMonsterBase::OnChase(float DeltaSeconds)
{
}

void AAIMonsterBase::OnAttack()
{
	UE_LOG(LogTemp, Warning,
		TEXT("=== OnAttack Start | Authority: %d ==="),
		HasAuthority()
	);

	if ( !HasAuthority() )
	{
		UE_LOG(LogTemp, Error, TEXT("Not Server!"));
		return;
	}

	float Now = GetWorld()->GetTimeSeconds();

	if ( Now - LastAttackTime < AttackCooldown )
	{
		UE_LOG(LogTemp, Warning, TEXT("Cooldown"));
		return;
	}

	AUK_AiMonsterCtl* AI =
		Cast<AUK_AiMonsterCtl>(GetController());

	if ( !AI )
	{
		UE_LOG(LogTemp, Error, TEXT("No AIController"));
		return;
	}

	AActor* Target = AI->GetCurrentTarget();

	UE_LOG(LogTemp, Warning,
		TEXT("Target: %s"),
		Target ? *Target->GetName() : TEXT("NULL")
	);

	if ( !Target ) return;

	float Dist = FVector::Dist(
		GetActorLocation(),
		Target->GetActorLocation()
	);

	UE_LOG(LogTemp, Warning,
		TEXT("Distance: %.1f / Range: %.1f"),
		Dist,
		AttackRange
	);

	if ( Dist > AttackRange )
	{
		UE_LOG(LogTemp, Warning, TEXT("Out of Range"));
		return;
	}

	AUK_CharacterBase* Player =
		Cast<AUK_CharacterBase>(Target);

	if ( !Player )
	{
		UE_LOG(LogTemp, Error,
			TEXT("Cast Failed: %s"),
			*Target->GetClass()->GetName()
		);
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("Before Damage"));

	Player->ReceiveDamage(AttackDamage);

	UE_LOG(LogTemp, Warning,
		TEXT("[Monster Attack] %s -> %s | Damage: %.1f"),
		*GetName(),
		*Player->GetName(),
		AttackDamage
	);

	LastAttackTime = Now;
}

void AAIMonsterBase::OnDead()
{
}

void AAIMonsterBase::OnPassive()
{
	// 평화로운 상태 - 풀 뜯기, 잠자기 등의 행동
	// 애니메이션은 블루프린트에서 처리
}

void AAIMonsterBase::OnAlert()
{
	// 경계 상태 - 으르렁, 뒷걸음질
	// 애니메이션은 블루프린트에서 처리
}

void AAIMonsterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AAIMonsterBase, CurrentState);
}