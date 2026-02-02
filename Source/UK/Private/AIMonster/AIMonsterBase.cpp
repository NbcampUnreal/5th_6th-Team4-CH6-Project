#include "AIMonster/AIMonsterBase.h"
#include "AIController.h"
#include "Net/UnrealNetwork.h"
#include "AIMonster/UK_AiMonsterCtl.h"
#include "Character/UK_CharacterBase.h"

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
    }
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
	// HP 컴포넌트 추가시 여기서 초기화
	// 현재는 상태만 리셋
	if (HasAuthority())
	{
		SetServerState(EMonsterState::Idle);
	}
}

void AAIMonsterBase::ReceiveDamage(float Damage)
{
	if ( !HasAuthority() ) return;

	if ( StatComponent )
	{
		StatComponent->TakeDamage(Damage);
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
	UE_LOG(LogTemp, Error, TEXT("ON Attack"));
	if ( !HasAuthority() ) return;
	
	/* 쿨타임 */

	float Now = GetWorld()->GetTimeSeconds();

	if ( Now - LastAttackTime < AttackCooldown )
		return;


	/* AIController */

	AUK_AiMonsterCtl* AI =
		Cast<AUK_AiMonsterCtl>(GetController());

	if ( !AI ) return;


	/* Target */

	AActor* Target = AI->GetCurrentTarget();

	if ( !Target ) return;


	/* 거리 체크 */

	float Dist = FVector::Dist(
		GetActorLocation(),
		Target->GetActorLocation()
	);

	if ( Dist > AttackRange )
		return;


	/* 데미지 */

	AUK_CharacterBase* Player =
		Cast<AUK_CharacterBase>(Target);

	if ( Player )
	{
		//Player->ReceiveDamage(AttackDamage);
	}


	/* 쿨타임 갱신 */

	LastAttackTime = Now;
}

void AAIMonsterBase::OnDead()
{
}

void AAIMonsterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AAIMonsterBase, CurrentState);
}
